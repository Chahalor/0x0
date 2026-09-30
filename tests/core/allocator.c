#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "core.h"

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct allocation_stats
{
	size_t allocations;
	size_t releases;
	int fail;
};

static struct allocation_stats *active_stats;

static void *tracked_alloc(size_t size)
{
	struct allocation_stats *stats = active_stats;

	stats->allocations++;
	if (stats->fail)
		return NULL;
	return malloc(size);
}

static void tracked_free(void *ptr)
{
	struct allocation_stats *stats = active_stats;

	stats->releases++;
	free(ptr);
}

static void *failed_realloc(void *ptr,
		size_t old_size, size_t new_size)
{
	(void)ptr;
	(void)old_size;
	(void)new_size;
	return NULL;
}

static void test_system_and_validation(void **state)
{
	t_genesis_core_allocator allocator = core_allocator_system();

	(void)state;
	assert_non_null(allocator.alloc);
	assert_non_null(allocator.realloc);
	assert_non_null(allocator.free);
	assert_int_equal(core_allocator_validate(&allocator), 0);
	assert_int_equal(core_allocator_validate(NULL), EINVAL);
	allocator.alloc = NULL;
	assert_int_equal(core_allocator_validate(&allocator), EINVAL);
	allocator = core_allocator_system();
	allocator.free = NULL;
	assert_int_equal(core_allocator_validate(&allocator), EINVAL);
	allocator = core_allocator_system();
	allocator.realloc = NULL;
	assert_int_equal(core_allocator_validate(&allocator), 0);
}

static void test_alloc_and_array_contracts(void **state)
{
	t_genesis_core_allocator allocator = core_allocator_system();
	unsigned char *bytes;

	(void)state;
	errno = 0;
	assert_null(core_alloc(NULL, 4));
	assert_int_equal(errno, EINVAL);
	errno = 0;
	assert_null(core_alloc(&allocator, 0));
	assert_int_equal(errno, EINVAL);
	bytes = core_alloc(&allocator, 4);
	assert_non_null(bytes);
	memcpy(bytes, "abcd", 4);
	assert_memory_equal(bytes, "abcd", 4);
	core_free(&allocator, bytes);
	errno = 0;
	assert_null(core_alloc_array(&allocator, 0, 4));
	assert_int_equal(errno, EINVAL);
	errno = 0;
	assert_null(core_alloc_array(&allocator, 4, 0));
	assert_int_equal(errno, EINVAL);
	errno = 0;
	assert_null(core_alloc_array(&allocator, SIZE_MAX, 2));
	assert_int_equal(errno, EOVERFLOW);
	bytes = core_alloc_array(&allocator, 2, 4);
	assert_non_null(bytes);
	core_free(&allocator, bytes);
}

static void test_calloc_and_failure(void **state)
{
	struct allocation_stats stats = {0};
	t_genesis_core_allocator allocator = {
		.alloc = tracked_alloc, .free = tracked_free
	};
	unsigned char *bytes;
	size_t index;

	(void)state;
	active_stats = &stats;
	errno = 0;
	assert_null(core_calloc(&allocator, 0, 4));
	assert_int_equal(errno, EINVAL);
	errno = 0;
	assert_null(core_calloc(&allocator, SIZE_MAX, 2));
	assert_int_equal(errno, EOVERFLOW);
	bytes = core_calloc(&allocator, 4, 2);
	assert_non_null(bytes);
	for (index = 0; index < 8; index++)
		assert_int_equal(bytes[index], 0);
	core_free(&allocator, bytes);
	stats.fail = 1;
	errno = 0;
	assert_null(core_alloc(&allocator, 2));
	assert_int_equal(errno, ENOMEM);
	assert_int_equal(stats.releases, 1);
}

static void test_realloc_contracts(void **state)
{
	t_genesis_core_allocator allocator = core_allocator_system();
	unsigned char *bytes;
	unsigned char *grown;

	(void)state;
	errno = 0;
	assert_null(core_realloc(NULL, NULL, 0, 1));
	assert_int_equal(errno, EINVAL);
	errno = 0;
	assert_null(core_realloc(&allocator, NULL, 0, 0));
	assert_int_equal(errno, EINVAL);
	bytes = core_realloc(&allocator, NULL, 0, 3);
	assert_non_null(bytes);
	memcpy(bytes, "abc", 3);
	grown = core_realloc(&allocator, bytes, 3, 9);
	assert_non_null(grown);
	assert_memory_equal(grown, "abc", 3);
	bytes = core_realloc(&allocator, grown, 9, 2);
	assert_non_null(bytes);
	assert_memory_equal(bytes, "ab", 2);
	assert_null(core_realloc(&allocator, bytes, 2, 0));
}

static void test_realloc_fallback_and_preserved_pointer(void **state)
{
	struct allocation_stats stats = {0};
	t_genesis_core_allocator allocator = {
		.alloc = tracked_alloc, .free = tracked_free
	};
	unsigned char *bytes;
	unsigned char *grown;

	(void)state;
	active_stats = &stats;
	bytes = core_alloc(&allocator, 3);
	assert_non_null(bytes);
	memcpy(bytes, "abc", 3);
	stats.fail = 1;
	errno = 0;
	assert_null(core_realloc(&allocator, bytes, 3, 8));
	assert_int_equal(errno, ENOMEM);
	assert_memory_equal(bytes, "abc", 3);
	stats.fail = 0;
	grown = core_realloc(&allocator, bytes, 3, 8);
	assert_non_null(grown);
	assert_memory_equal(grown, "abc", 3);
	core_free(&allocator, grown);
	assert_int_equal(stats.allocations, 3);
	assert_int_equal(stats.releases, 2);
	allocator.realloc = failed_realloc;
	bytes = core_alloc(&allocator, 2);
	assert_non_null(bytes);
	memcpy(bytes, "xy", 2);
	errno = 0;
	assert_null(core_realloc(&allocator, bytes, 2, 4));
	assert_int_equal(errno, ENOMEM);
	assert_memory_equal(bytes, "xy", 2);
	core_free(&allocator, bytes);
}

static void test_free_contracts(void **state)
{
	t_genesis_core_allocator allocator = core_allocator_system();
	void *ptr;

	(void)state;
	core_free(NULL, NULL);
	ptr = core_alloc(&allocator, 4);
	assert_non_null(ptr);
	errno = 0;
	core_free(NULL, ptr);
	assert_int_equal(errno, EINVAL);
	core_free(&allocator, ptr);
}

int main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_system_and_validation),
		cmocka_unit_test(test_alloc_and_array_contracts),
		cmocka_unit_test(test_calloc_and_failure),
		cmocka_unit_test(test_realloc_contracts),
		cmocka_unit_test(test_realloc_fallback_and_preserved_pointer),
		cmocka_unit_test(test_free_contracts)
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
