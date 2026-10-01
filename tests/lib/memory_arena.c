#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "core/setting/setting.h"

static void test_capacity_reset_and_trim(void **state)
{
	MEM_ARENA *arena;
	unsigned char *bytes;

	(void)state;
	arena = mem_arena_new();
	assert_non_null(arena);
	bytes = mem_arena_alloc(arena, 64u * 1024u);
	assert_non_null(bytes);
	assert_int_equal(bytes[0], 0);
	assert_int_equal(bytes[64u * 1024u - 1], 0);
	errno = 0;
	assert_null(mem_arena_alloc(arena, 1));
	assert_int_equal(errno, ENOMEM);
	mem_arena_reset(arena);
	mem_arena_trim(arena);
	assert_non_null(mem_arena_alloc(arena, 32));
	assert_int_equal((uintptr_t)mem_arena_alloc(arena, 1)
		% _Alignof(max_align_t), 0);
	mem_arena_free(arena);
}

static void test_rewind_preserves_before_mark(void **state)
{
	MEM_ARENA *arena;
	MEM_ARENA *other;
	unsigned char *before;
	unsigned char *after;
	unsigned char *again;
	t_arena_mark first;
	t_arena_mark later;
	t_arena_mark replacement;

	(void)state;
	arena = mem_arena_new();
	assert_non_null(arena);
	other = mem_arena_new();
	assert_non_null(other);
	before = mem_arena_alloc(arena, 32);
	assert_non_null(before);
	memset(before, 0x5a, 32);
	first = mem_arena_mark(arena);
	assert_non_null(first);
	after = mem_arena_alloc(arena, 32);
	assert_non_null(after);
	memset(after, 0xa5, 32);
	later = mem_arena_mark(arena);
	assert_non_null(later);
	assert_non_null(mem_arena_alloc(arena, 32));
	errno = 0;
	mem_arena_rewind(other, first);
	assert_int_equal(errno, EINVAL);
	assert_int_equal(after[0], 0xa5);
	mem_arena_rewind(arena, first);
	assert_int_equal(before[0], 0x5a);
	assert_int_equal(before[31], 0x5a);
	again = mem_arena_alloc(arena, 32);
	assert_ptr_equal(again, after);
	assert_int_equal(again[0], 0);
	errno = 0;
	mem_arena_rewind(arena, later);
	assert_int_equal(errno, EINVAL);
	replacement = mem_arena_mark(arena);
	assert_non_null(replacement);
	assert_true(replacement != later);
	errno = 0;
	mem_arena_rewind(arena, later);
	assert_int_equal(errno, EINVAL);
	mem_arena_rewind(arena, first);
	assert_int_equal(before[0], 0x5a);
	mem_arena_free(other);
	mem_arena_free(arena);
}

static void test_dup_and_invalid_arguments(void **state)
{
	MEM_ARENA *arena;
	char *tracked;
	char *copy;
	char text[] = "abc";

	(void)state;
	arena = mem_arena_new();
	assert_non_null(arena);
	copy = mem_arena_dup(arena, text, 4);
	assert_non_null(copy);
	assert_string_equal(copy, "abc");
	tracked = mem_dup(text, 4);
	assert_non_null(tracked);
	copy = mem_arena_dup(arena, tracked, -1);
	assert_non_null(copy);
	assert_string_equal(copy, "abc");
	mem_free(tracked);
	errno = 0;
	assert_null(mem_arena_dup(arena, text, -1));
	assert_int_equal(errno, EINVAL);
	errno = 0;
	assert_null(mem_arena_alloc(arena, 0));
	assert_int_equal(errno, EINVAL);
	errno = 0;
	assert_null(mem_arena_dup(arena, NULL, 4));
	assert_int_equal(errno, EINVAL);
	mem_arena_free(arena);
}

struct allocator_counts
{
	size_t allocations;
	size_t releases;
	size_t fail_at;
	size_t failures;
};

static struct allocator_counts *active_counts;

static void *counted_alloc(size_t size)
{
	active_counts->allocations++;
	if (active_counts->allocations == active_counts->fail_at)
	{
		active_counts->failures++;
		return (NULL);
	}
	return (malloc(size));
}

static void counted_free(void *ptr)
{
	active_counts->releases++;
	free(ptr);
}

static void test_arena_uses_current_global_allocator(void **state)
{
	t_genesis_global_settings saved = G_SETTINGS;
	struct allocator_counts counts = {0};
	MEM_ARENA *arena;

	(void)state;
	mem_free_all();
	arena = mem_arena_new();
	assert_non_null(arena);
	active_counts = &counts;
	G_SETTINGS.allocator.alloc = counted_alloc;
	G_SETTINGS.allocator.free = counted_free;
	assert_non_null(mem_arena_mark(arena));
	mem_arena_reset(arena);
	mem_arena_trim(arena);
	assert_non_null(mem_arena_alloc(arena, 8));
	mem_free_all();
	assert_true(counts.allocations >= 2);
	assert_true(counts.releases >= counts.allocations);
	G_SETTINGS = saved;
}

static void test_arena_allocation_failures(void **state)
{
	t_genesis_global_settings saved = G_SETTINGS;
	struct allocator_counts counts = {0};
	MEM_ARENA *arena;

	(void)state;
	mem_free_all();
	active_counts = &counts;
	G_SETTINGS.allocator.alloc = counted_alloc;
	G_SETTINGS.allocator.free = counted_free;
	counts.fail_at = 1;
	errno = 0;
	assert_null(mem_arena_new());
	assert_int_equal(errno, ENOMEM);
	counts.fail_at = 3;
	errno = 0;
	assert_null(mem_arena_new());
	assert_int_equal(errno, ENOMEM);
	assert_int_equal(counts.allocations - counts.failures, counts.releases);
	counts.fail_at = 0;
	arena = mem_arena_new();
	assert_non_null(arena);
	counts.fail_at = counts.allocations + 1;
	errno = 0;
	assert_null(mem_arena_mark(arena));
	assert_int_equal(errno, ENOMEM);
	mem_arena_reset(arena);
	mem_arena_trim(arena);
	counts.fail_at = counts.allocations + 1;
	errno = 0;
	assert_null(mem_arena_alloc(arena, 8));
	assert_int_equal(errno, ENOMEM);
	mem_arena_free(arena);
	assert_int_equal(counts.allocations - counts.failures, counts.releases);
	G_SETTINGS = saved;
}

int main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_capacity_reset_and_trim),
		cmocka_unit_test(test_rewind_preserves_before_mark),
		cmocka_unit_test(test_dup_and_invalid_arguments),
		cmocka_unit_test(test_arena_uses_current_global_allocator),
		cmocka_unit_test(test_arena_allocation_failures)
	};

	return (cmocka_run_group_tests(tests, NULL, NULL));
}
