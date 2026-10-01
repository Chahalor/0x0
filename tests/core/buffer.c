#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "core/buffer/buffer.h"
#include "core/buffer/error.h"
#include "core/setting/setting.h"

static void *failed_alloc(size_t size)
{
	(void)size;
	return (NULL);
}

static void *failed_realloc(void *ptr, size_t size)
{
	(void)ptr;
	(void)size;
	return (NULL);
}

static size_t allocation_calls;
static size_t release_calls;

static void *fail_second_alloc(size_t size)
{
	allocation_calls++;
	if (allocation_calls == 2)
		return (NULL);
	return (malloc(size));
}

static void count_free(void *ptr)
{
	release_calls++;
	free(ptr);
}

static void *moving_realloc(void *ptr, size_t size)
{
	void *replacement = malloc(size);

	if (!replacement)
		return (NULL);
	if (ptr)
	{
		memcpy(replacement, ptr, 2);
		free(ptr);
	}
	return (replacement);
}

static void test_init_zero_and_release(void **state)
{
	t_genesis_core_buffer buffer = {0};
	t_genesis_core_buffer *owned;

	(void)state;
	assert_int_equal(core_buffer_init(NULL, 2), EINVAL);
	assert_int_equal(core_buffer_init(&buffer, 0), 0);
	assert_null(buffer.bytes);
	assert_int_equal(buffer.capacity, 0);
	assert_int_equal(core_buffer_append(&buffer, "ab", 2), 0);
	assert_memory_equal(buffer.bytes, "ab", 2);
	core_buffer_deinit(&buffer);
	assert_null(buffer.bytes);
	assert_int_equal(buffer.length, 0);
	assert_int_equal(buffer.capacity, 0);
	owned = core_buffer_new(0);
	assert_non_null(owned);
	assert_null(owned->bytes);
	core_buffer_free(owned);
}

static void test_growth_and_self_append(void **state)
{
	t_genesis_core_buffer buffer = {0};

	(void)state;
	assert_int_equal(core_buffer_init(&buffer, 2), 0);
	assert_int_equal(core_buffer_append(&buffer, "ab", 2), 0);
	assert_int_equal(core_buffer_append(&buffer, buffer.bytes, 2), 0);
	assert_memory_equal(buffer.bytes, "abab", 4);
	assert_int_equal(core_buffer_resize(&buffer, 6), 0);
	assert_int_equal(buffer.bytes[4], 0);
	assert_int_equal(buffer.bytes[5], 0);
	core_buffer_clear(&buffer);
	assert_int_equal(buffer.length, 0);
	core_buffer_deinit(&buffer);
}

static void test_self_append_after_pointer_moves(void **state)
{
	t_genesis_core_buffer buffer = {0};
	void *(*old_realloc)(void *, size_t) = G_SETTINGS.allocator.realloc;

	(void)state;
	assert_int_equal(core_buffer_init(&buffer, 2), 0);
	assert_int_equal(core_buffer_append(&buffer, "ab", 2), 0);
	G_SETTINGS.allocator.realloc = moving_realloc;
	assert_int_equal(core_buffer_append(&buffer, buffer.bytes, 2), 0);
	assert_memory_equal(buffer.bytes, "abab", 4);
	G_SETTINGS.allocator.realloc = old_realloc;
	core_buffer_deinit(&buffer);
}

static void test_fixed_append_and_invalid_state(void **state)
{
	t_genesis_core_buffer buffer = {0};
	t_genesis_core_buffer invalid = {.bytes = NULL, .length = 2, .capacity = 1};

	(void)state;
	assert_int_equal(core_buffer_reserve(NULL, 2), EINVAL);
	assert_int_equal(core_buffer_resize(NULL, 2), EINVAL);
	assert_int_equal(core_buffer_append(NULL, "a", 1), EINVAL);
	assert_int_equal(core_buffer_append_fixed(NULL, "a", 1), EINVAL);
	assert_int_equal(core_buffer_append(&invalid, "a", 1), EINVAL);
	core_buffer_deinit(NULL);
	core_buffer_free(NULL);
	core_buffer_clear(NULL);
	assert_int_equal(core_buffer_init(&buffer, 3), 0);
	assert_int_equal(core_buffer_append_fixed(&buffer, "ab", 2), 0);
	assert_int_equal(core_buffer_append_fixed(&buffer, "cd", 2), ENOSPC);
	assert_int_equal(core_buffer_append_fixed(&buffer, buffer.bytes, 1), 0);
	assert_memory_equal(buffer.bytes, "aba", 3);
	assert_int_equal(buffer.capacity, 3);
	core_buffer_deinit(&buffer);
}

static void test_invalid_source_and_overflow(void **state)
{
	t_genesis_core_buffer buffer = {0};

	(void)state;
	assert_int_equal(core_buffer_init(&buffer, 8), 0);
	assert_int_equal(core_buffer_append(&buffer, "abc", 3), 0);
	assert_int_equal(core_buffer_append(&buffer, buffer.bytes + 4, 1),
		-BUFFER_ERROR_SOURCE_OUT_OF_RANGE);
	assert_int_equal(core_buffer_append(&buffer, NULL, 1), EINVAL);
	assert_int_equal(core_buffer_append(&buffer, "x", SIZE_MAX), EOVERFLOW);
	assert_int_equal(core_buffer_append_fixed(&buffer, "123456", 6), ENOSPC);
	assert_int_equal(buffer.length, 3);
	core_buffer_deinit(&buffer);
}

static void test_allocation_failure_is_atomic(void **state)
{
	t_genesis_core_buffer buffer = {0};
	void *(*old_alloc)(size_t) = G_SETTINGS.allocator.alloc;
	void *(*old_realloc)(void *, size_t) = G_SETTINGS.allocator.realloc;

	(void)state;
	G_SETTINGS.allocator.alloc = failed_alloc;
	assert_int_equal(core_buffer_init(&buffer, 4), ENOMEM);
	assert_null(buffer.bytes);
	assert_null(core_buffer_new(4));
	G_SETTINGS.allocator.alloc = old_alloc;
	assert_int_equal(core_buffer_init(&buffer, 4), 0);
	G_SETTINGS.allocator.realloc = failed_realloc;
	assert_int_equal(core_buffer_reserve(&buffer, 16), ENOMEM);
	assert_int_equal(buffer.capacity, 4);
	assert_non_null(buffer.bytes);
	G_SETTINGS.allocator.realloc = old_realloc;
	core_buffer_deinit(&buffer);
}

static void test_new_second_allocation_failure(void **state)
{
	void *(*old_alloc)(size_t) = G_SETTINGS.allocator.alloc;
	void (*old_free)(void *) = G_SETTINGS.allocator.free;

	(void)state;
	allocation_calls = 0;
	release_calls = 0;
	G_SETTINGS.allocator.alloc = fail_second_alloc;
	G_SETTINGS.allocator.free = count_free;
	errno = 0;
	assert_null(core_buffer_new(4));
	assert_int_equal(errno, ENOMEM);
	assert_int_equal(allocation_calls, 2);
	assert_int_equal(release_calls, 1);
	G_SETTINGS.allocator.alloc = old_alloc;
	G_SETTINGS.allocator.free = old_free;
}

int main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_init_zero_and_release),
		cmocka_unit_test(test_growth_and_self_append),
		cmocka_unit_test(test_self_append_after_pointer_moves),
		cmocka_unit_test(test_fixed_append_and_invalid_state),
		cmocka_unit_test(test_invalid_source_and_overflow),
		cmocka_unit_test(test_allocation_failure_is_atomic),
		cmocka_unit_test(test_new_second_allocation_failure)
	};

	return (cmocka_run_group_tests(tests, NULL, NULL));
}
