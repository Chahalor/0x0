#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "lib/memory/_internal_/_memory.h"

struct fail_allocator
{
	size_t calls;
	size_t releases;
	size_t fail_at;
};

#if MEM_MANAGER_TYPE == 1
static struct fail_allocator *active_fail_allocator;

static void *fail_alloc(size_t size)
{
	struct fail_allocator *stats = active_fail_allocator;

	stats->calls++;
	if (stats->calls == stats->fail_at)
		return NULL;
	return malloc(size);
}

static void fail_free(void *ptr)
{
	struct fail_allocator *stats = active_fail_allocator;

	stats->releases++;
	free(ptr);
}

static void test_tree_allocation_failures(void **state)
{
	struct fail_allocator stats = {0};
	t_genesis_core_allocator allocator = {fail_alloc, NULL, fail_free};
	rb_tree *tree;

	(void)state;
	active_fail_allocator = &stats;
	stats.fail_at = 1;
	assert_null(rb_init(&allocator));
	assert_int_equal(stats.releases, 0);
	stats.fail_at = 3;
	assert_null(rb_init(&allocator));
	assert_int_equal(stats.releases, 1);
	stats.fail_at = 0;
	tree = rb_init(&allocator);
	assert_non_null(tree);
	stats.fail_at = stats.calls + 1;
	assert_null(rb_create_node(tree, 8));
	assert_int_equal(errno, ENOMEM);
	core_free(&tree->allocator, tree->nil);
	core_free(&tree->allocator, tree);
	assert_int_equal(stats.releases, 3);
}
#endif

static void test_free_unregisters_and_rejects_unknown(void **state)
{
	char *bytes;
	char foreign = 'x';

	(void)state;
	bytes = mem_alloc(4);
	assert_non_null(bytes);
	memcpy(bytes, "abc", 4);
	mem_free(bytes);
	errno = 0;
	assert_null(mem_realloc(bytes, 8));
	assert_int_equal(errno, EINVAL);
	errno = 0;
	mem_free(&foreign);
	assert_int_equal(errno, EINVAL);
	mem_free(NULL);
}

static void test_realloc_preserves_data_and_reports_result(void **state)
{
	char *bytes;
	char *grown;

	(void)state;
	bytes = mem_alloc(4);
	assert_non_null(bytes);
	memcpy(bytes, "abc", 4);
	grown = mem_realloc(bytes, 16);
	assert_non_null(grown);
	assert_memory_equal(grown, "abc", 4);
	mem_free(grown);
}

static void test_realloc_failure_keeps_old_block(void **state)
{
	char *bytes;

	(void)state;
	bytes = mem_alloc(4);
	assert_non_null(bytes);
	memcpy(bytes, "abc", 4);
	errno = 0;
	assert_null(mem_realloc(bytes, SIZE_MAX));
	assert_int_equal(errno, EOVERFLOW);
	assert_memory_equal(bytes, "abc", 4);
	mem_free(bytes);
}

static void test_dup_join_and_boundaries(void **state)
{
	char *bytes;
	char *copy;
	char *joined;
	char foreign[] = "xy";

	(void)state;
	bytes = mem_dup("ab", 2);
	assert_non_null(bytes);
	copy = mem_dup(bytes, -1);
	assert_non_null(copy);
	assert_memory_equal(copy, "ab", 2);
	joined = mem_join(bytes, foreign, -1, 2);
	assert_non_null(joined);
	assert_memory_equal(joined, "abxy", 4);
	errno = 0;
	assert_null(mem_dup(bytes, 3));
	assert_int_equal(errno, EINVAL);
	errno = 0;
	assert_null(mem_join(bytes, foreign, 3, 2));
	assert_int_equal(errno, EINVAL);
	errno = 0;
	assert_null(mem_dup(bytes, 0));
	assert_int_equal(errno, EINVAL);
	errno = 0;
	assert_null(mem_join(bytes, foreign, 0, 2));
	assert_int_equal(errno, EINVAL);
	mem_free(joined);
	mem_free(copy);
	mem_free(bytes);
	errno = 0;
	assert_null(mem_dup(foreign, -1));
	assert_int_equal(errno, EINVAL);
	errno = 0;
	assert_null(mem_join(foreign, foreign, -1, 2));
	assert_int_equal(errno, EINVAL);
}

static void test_zero_and_reset(void **state)
{
	char *bytes;

	(void)state;
	errno = 0;
	assert_null(mem_alloc(0));
	assert_int_equal(errno, EINVAL);
	errno = 0;
	assert_null(mem_realloc(NULL, 0));
	assert_int_equal(errno, EINVAL);
	bytes = mem_realloc(NULL, 4);
	assert_non_null(bytes);
	mem_free(bytes);
	bytes = mem_alloc(4);
	assert_non_null(bytes);
	errno = 0;
	assert_null(mem_realloc(bytes, 0));
	assert_int_equal(errno, 0);
	errno = 0;
	assert_null(mem_realloc(bytes, 4));
	assert_int_equal(errno, EINVAL);
	bytes = mem_alloc(4);
	assert_non_null(bytes);
	mem_free_all();
	mem_free_all();
	bytes = mem_alloc(4);
	assert_non_null(bytes);
	mem_free(bytes);
}

static void test_many_deletions(void **state)
{
	void *blocks[256];
	size_t i;

	(void)state;
	i = 0;
	while (i < 256)
	{
		blocks[i] = mem_alloc(i + 1);
		assert_non_null(blocks[i]);
		i++;
	}
	i = 0;
	while (i < 256)
	{
		mem_free(blocks[(i * 73) % 256]);
		i++;
	}
	mem_free_all();
}

int main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_free_unregisters_and_rejects_unknown),
		cmocka_unit_test(test_realloc_preserves_data_and_reports_result),
		cmocka_unit_test(test_realloc_failure_keeps_old_block),
		cmocka_unit_test(test_dup_join_and_boundaries),
		cmocka_unit_test(test_zero_and_reset),
		cmocka_unit_test(test_many_deletions),
		#if MEM_MANAGER_TYPE == 1
		cmocka_unit_test(test_tree_allocation_failures),
		#endif
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
