#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include "core.h"

static void *custom_alloc(size_t size)
{
	return (malloc(size));
}

static void custom_free(void *ptr)
{
	free(ptr);
}

static void *custom_realloc(void *ptr, size_t size)
{
	return (realloc(ptr, size));
}

static void test_global_allocator_callbacks(void **state)
{
	void *ptr;

	(void)state;
	assert_non_null(G_SETTINGS.allocator.alloc);
	assert_non_null(G_SETTINGS.allocator.free);
	assert_non_null(G_SETTINGS.allocator.realloc);
	G_SETTINGS.allocator.alloc = custom_alloc;
	G_SETTINGS.allocator.free = custom_free;
	G_SETTINGS.allocator.realloc = custom_realloc;
	ptr = G_SETTINGS.allocator.alloc(8);
	assert_non_null(ptr);
	ptr = G_SETTINGS.allocator.realloc(ptr, 16);
	assert_non_null(ptr);
	G_SETTINGS.allocator.free(ptr);
	G_SETTINGS.allocator.alloc = malloc;
	G_SETTINGS.allocator.free = free;
	G_SETTINGS.allocator.realloc = realloc;
}

int main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_global_allocator_callbacks)
	};

	return (cmocka_run_group_tests(tests, NULL, NULL));
}
