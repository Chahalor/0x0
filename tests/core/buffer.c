#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "core.h"

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static void *failed_alloc(void *context, size_t size)
{
	(void)context;
	(void)size;
	return NULL;
}

static void plain_free(void *context, void *ptr, size_t size)
{
	(void)context;
	(void)size;
	free(ptr);
}

struct failure_stats
{
	size_t allocations;
	size_t releases;
	size_t fail_at;
};

static void *fail_after_alloc(void *context, size_t size)
{
	struct failure_stats *stats = context;

	stats->allocations++;
	if (stats->allocations == stats->fail_at)
		return NULL;
	return malloc(size);
}

static void fail_after_free(void *context, void *ptr, size_t size)
{
	struct failure_stats *stats = context;

	(void)size;
	stats->releases++;
	free(ptr);
}

struct move_stats
{
	size_t moves;
};

static void *moving_alloc(void *context, size_t size)
{
	(void)context;
	return malloc(size);
}

static void *moving_realloc(void *context, void *ptr,
		size_t old_size, size_t new_size)
{
	struct move_stats *stats = context;
	void *replacement = malloc(new_size);

	if (!replacement)
		return NULL;
	memcpy(replacement, ptr, old_size < new_size ? old_size : new_size);
	free(ptr);
	stats->moves++;
	return replacement;
}

static void test_init_deinit_and_invalid_arguments(void **state)
{
	t_genesis_core_buffer buffer;
	t_genesis_core_allocator allocator = core_allocator_system();
	t_genesis_settings settings;

	(void)state;
	assert_int_equal(core_buffer_init(NULL, 4), EINVAL);
	assert_int_equal(core_buffer_init(&buffer, 0), 0);
	assert_null(buffer.bytes);
	assert_int_equal(buffer.length, 0);
	assert_int_equal(buffer.capacity, 0);
	core_buffer_deinit(&buffer);
	assert_null(buffer.bytes);
	core_buffer_deinit(NULL);
	buffer.length = 123;
	assert_int_equal(core_buffer_init_with_allocator(NULL, 4, &allocator),
		EINVAL);
	assert_int_equal(core_buffer_init_with_allocator(&buffer, 4, NULL),
		EINVAL);
	assert_int_equal(buffer.length, 123);
	assert_int_equal(core_buffer_init_with_allocator(&buffer, 4,
		&allocator), 0);
	assert_int_equal(buffer.capacity, 4);
	core_buffer_deinit(&buffer);
	assert_int_equal(core_buffer_init_with_settings(&buffer, 0, NULL),
		EINVAL);
	assert_int_equal(buffer.length, 0);
	assert_int_equal(core_buffer_init_with_settings(NULL, 0, &settings),
		EINVAL);
	assert_int_equal(genesis_settings_init(&settings), 0);
	assert_int_equal(core_buffer_init_with_settings(&buffer, 0, &settings),
		0);
	assert_int_equal(buffer.settings.initial_capacity, 64);
	assert_int_equal(buffer.settings.growth, BUFFER_GROWTH_GEOMETRIC);
	core_buffer_deinit(&buffer);
	genesis_settings_deinit(&settings);
}

static void test_owned_constructors_and_failure(void **state)
{
	t_genesis_core_allocator allocator = core_allocator_system();
	t_genesis_core_allocator failing = {
		.alloc = failed_alloc, .free = plain_free
	};
	struct failure_stats failure = { .fail_at = 2 };
	t_genesis_core_allocator fail_second = {
		.context = &failure, .alloc = fail_after_alloc,
		.free = fail_after_free
	};
	t_genesis_settings settings;
	t_genesis_core_buffer *buffer;

	(void)state;
	buffer = core_buffer_new(0);
	assert_non_null(buffer);
	assert_int_equal(buffer->capacity, 0);
	core_buffer_free(buffer);
	core_buffer_free(NULL);
	errno = 0;
	assert_null(core_buffer_new_with_allocator(4, NULL));
	assert_int_equal(errno, EINVAL);
	buffer = core_buffer_new_with_allocator(4, &allocator);
	assert_non_null(buffer);
	assert_int_equal(buffer->capacity, 4);
	core_buffer_free(buffer);
	errno = 0;
	assert_null(core_buffer_new_with_allocator(4, &failing));
	assert_int_equal(errno, ENOMEM);
	errno = 0;
	assert_null(core_buffer_new_with_allocator(4, &fail_second));
	assert_int_equal(errno, ENOMEM);
	assert_int_equal(failure.allocations, 2);
	assert_int_equal(failure.releases, 1);
	errno = 0;
	assert_null(core_buffer_new_with_settings(0, NULL));
	assert_int_equal(errno, EINVAL);
	assert_int_equal(genesis_settings_init(&settings), 0);
	buffer = core_buffer_new_with_settings(0, &settings);
	assert_non_null(buffer);
	core_buffer_free(buffer);
	genesis_settings_deinit(&settings);
}

static void test_init_allocation_failure(void **state)
{
	struct failure_stats failure = { .fail_at = 1 };
	t_genesis_core_allocator allocator = {
		.context = &failure, .alloc = fail_after_alloc,
		.free = fail_after_free
	};
	t_genesis_core_buffer buffer = { .length = 123 };
	t_genesis_settings settings;

	(void)state;
	assert_int_equal(core_buffer_init_with_allocator(&buffer, 4,
		&allocator), ENOMEM);
	assert_null(buffer.bytes);
	assert_int_equal(buffer.length, 123);
	assert_int_equal(failure.allocations, 1);
	assert_int_equal(failure.releases, 0);
	assert_int_equal(genesis_settings_init(&settings), 0);
	assert_int_equal(genesis_settings_set(&settings,
		&genesis_allocator_setting, &allocator, sizeof(allocator)), 0);
	failure.fail_at = 2;
	assert_int_equal(core_buffer_init_with_settings(&buffer, 4, &settings),
		ENOMEM);
	assert_int_equal(buffer.length, 123);
	assert_int_equal(failure.allocations, 2);
	genesis_settings_deinit(&settings);
}

static void test_reserve_resize_and_clear(void **state)
{
	t_genesis_core_buffer buffer;
	size_t old_capacity;

	(void)state;
	assert_int_equal(core_buffer_reserve(NULL, 1), EINVAL);
	assert_int_equal(core_buffer_resize(NULL, 1), EINVAL);
	assert_int_equal(core_buffer_init(&buffer, 0), 0);
	assert_int_equal(core_buffer_reserve(&buffer, 1), 0);
	assert_int_equal(buffer.capacity, 64);
	assert_int_equal(buffer.length, 0);
	old_capacity = buffer.capacity;
	assert_int_equal(core_buffer_reserve(&buffer, 2), 0);
	assert_int_equal(buffer.capacity, old_capacity);
	assert_int_equal(core_buffer_resize(&buffer, 3), 0);
	assert_int_equal(buffer.length, 3);
	assert_memory_equal(buffer.bytes, "\0\0\0", 3);
	memcpy(buffer.bytes, "abc", 3);
	assert_int_equal(core_buffer_resize(&buffer, 1), 0);
	assert_int_equal(buffer.capacity, old_capacity);
	assert_int_equal(core_buffer_resize(&buffer, 4), 0);
	assert_memory_equal(buffer.bytes, "a\0\0\0", 4);
	core_buffer_clear(&buffer);
	assert_int_equal(buffer.length, 0);
	assert_int_equal(buffer.capacity, old_capacity);
	core_buffer_clear(NULL);
	core_buffer_deinit(&buffer);
}

static void test_growth_policy_snapshot(void **state)
{
	t_genesis_settings settings;
	t_genesis_core_buffer_settings growth = {
		.initial_capacity = 8, .growth = BUFFER_GROWTH_EXACT
	};
	t_genesis_core_buffer first;
	t_genesis_core_buffer second;

	(void)state;
	assert_int_equal(genesis_settings_init(&settings), 0);
	assert_int_equal(genesis_settings_set(&settings,
		&genesis_buffer_setting, &growth, sizeof(growth)), 0);
	assert_int_equal(core_buffer_init_with_settings(&first, 0, &settings),
		0);
	assert_int_equal(core_buffer_reserve(&first, 3), 0);
	assert_int_equal(first.capacity, 3);
	growth.growth = BUFFER_GROWTH_GEOMETRIC;
	assert_int_equal(genesis_settings_set(&settings,
		&genesis_buffer_setting, &growth, sizeof(growth)), 0);
	assert_int_equal(core_buffer_init_with_settings(&second, 0, &settings),
		0);
	assert_int_equal(core_buffer_reserve(&second, 3), 0);
	assert_int_equal(second.capacity, 8);
	assert_int_equal(first.settings.growth, BUFFER_GROWTH_EXACT);
	core_buffer_deinit(&first);
	core_buffer_deinit(&second);
	genesis_settings_deinit(&settings);
}

static void test_append_boundaries(void **state)
{
	t_genesis_core_buffer buffer;

	(void)state;
	assert_int_equal(core_buffer_append(NULL, "a", 1), EINVAL);
	assert_int_equal(core_buffer_append_fixed(NULL, "a", 1), EINVAL);
	assert_int_equal(core_buffer_init(&buffer, 4), 0);
	assert_int_equal(core_buffer_append(&buffer, NULL, 0), 0);
	assert_int_equal(core_buffer_append_fixed(&buffer, NULL, 0), 0);
	assert_int_equal(core_buffer_append(&buffer, NULL, 1), EINVAL);
	assert_int_equal(core_buffer_append_fixed(&buffer, NULL, 1), EINVAL);
	assert_int_equal(core_buffer_append_fixed(&buffer, "abcd", 4), 0);
	assert_int_equal(core_buffer_append_fixed(&buffer, "e", 1), ENOSPC);
	assert_int_equal(buffer.length, 4);
	assert_memory_equal(buffer.bytes, "abcd", 4);
	assert_int_equal(core_buffer_append(&buffer, "e", SIZE_MAX), EOVERFLOW);
	assert_int_equal(core_buffer_append_fixed(&buffer, "e", SIZE_MAX),
		EOVERFLOW);
	assert_int_equal(buffer.length, 4);
	core_buffer_deinit(&buffer);
}

/* Regression: a self-append must survive realloc moving the source bytes. */
static void test_append_from_own_storage(void **state)
{
	struct move_stats moves = {0};
	t_genesis_core_allocator allocator = {
		.context = &moves, .alloc = moving_alloc,
		.realloc = moving_realloc, .free = plain_free
	};
	t_genesis_core_buffer buffer;

	(void)state;
	assert_int_equal(core_buffer_init_with_allocator(&buffer, 2,
		&allocator), 0);
	assert_int_equal(core_buffer_append(&buffer, "ab", 2), 0);
	assert_int_equal(core_buffer_append(&buffer, buffer.bytes, 2), 0);
	assert_int_equal(moves.moves, 1);
	assert_memory_equal(buffer.bytes, "abab", 4);
	assert_int_equal(core_buffer_append(&buffer, buffer.bytes + 1, 2), 0);
	assert_memory_equal(buffer.bytes, "ababba", 6);
	core_buffer_deinit(&buffer);
}

/* Regression: bytes inside capacity but outside length are not a source. */
static void test_reject_internal_source_outside_length(void **state)
{
	t_genesis_core_buffer buffer;

	(void)state;
	assert_int_equal(core_buffer_init(&buffer, 8), 0);
	assert_int_equal(core_buffer_append(&buffer, "ab", 2), 0);
	assert_int_equal(core_buffer_append(&buffer, buffer.bytes + 1, 2),
		-BUFFER_ERROR_SOURCE_OUT_OF_RANGE);
	assert_int_equal(core_buffer_append_fixed(&buffer,
		buffer.bytes + 1, 2), -BUFFER_ERROR_SOURCE_OUT_OF_RANGE);
	assert_int_equal(buffer.length, 2);
	assert_memory_equal(buffer.bytes, "ab", 2);
	assert_int_equal(core_buffer_append_fixed(&buffer,
		buffer.bytes, 2), 0);
	assert_memory_equal(buffer.bytes, "abab", 4);
	core_buffer_deinit(&buffer);
}

static void test_invalid_buffer_invariants(void **state)
{
	t_genesis_core_buffer buffer;

	(void)state;
	assert_int_equal(core_buffer_init(&buffer, 2), 0);
	buffer.length = 3;
	assert_int_equal(core_buffer_reserve(&buffer, 4), EINVAL);
	assert_int_equal(core_buffer_resize(&buffer, 1), EINVAL);
	assert_int_equal(core_buffer_append(&buffer, "a", 1), EINVAL);
	buffer.length = 0;
	core_buffer_deinit(&buffer);
}

int main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_init_deinit_and_invalid_arguments),
		cmocka_unit_test(test_owned_constructors_and_failure),
		cmocka_unit_test(test_init_allocation_failure),
		cmocka_unit_test(test_reserve_resize_and_clear),
		cmocka_unit_test(test_growth_policy_snapshot),
		cmocka_unit_test(test_append_boundaries),
		cmocka_unit_test(test_append_from_own_storage),
		cmocka_unit_test(test_reject_internal_source_outside_length),
		cmocka_unit_test(test_invalid_buffer_invariants)
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
