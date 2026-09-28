#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <errno.h>
#include <string.h>

#include "types.h"
#include "types/setting.h"

static void *fail_alloc(size_t size)
{
	(void)size;
	errno = ENOMEM;
	return NULL;
}

static void test_setup_destroy_and_ownership(void **state)
{
	t_hash_map map = {0};
	char key[] = "first";
	int value = 7;
	void *(*old_alloc)(size_t);

	(void)state;
	assert_int_equal(hash_map_setup(NULL, 4), error_invalid_arg);
	assert_int_equal(hash_map_setup(&map, 0), error_invalid_arg);
	old_alloc = g_types_settings.alloc;
	g_types_settings.alloc = fail_alloc;
	assert_int_equal(hash_map_setup(&map, 4), ENOMEM);
	g_types_settings.alloc = old_alloc;
	assert_null(map.map);
	assert_int_equal(map.capacity, 0);
	assert_int_equal(hash_map_setup(&map, 4), 0);
	assert_int_equal(hash_map_add(&map, key, &value), 0);
	key[0] = 'x';
	assert_ptr_equal(hash_map_find(&map, "first"), &value);
	assert_int_equal(hash_map_destroy(&map), 0);
	assert_null(map.map);
	assert_int_equal(map.capacity, 0);
	assert_int_equal(map.length, 0);
	assert_int_equal(value, 7);
	assert_int_equal(hash_map_destroy(&map), 0);
}

static void test_duplicate_key_and_replace(void **state)
{
	t_hash_map map = {0};
	int first = 1;
	int second = 2;

	(void)state;
	assert_int_equal(hash_map_setup(&map, 2), 0);
	assert_int_equal(hash_map_add(&map, "key", &first), 0);
	assert_int_equal(hash_map_add(&map, "key", &second), EEXIST);
	assert_int_equal(map.length, 1);
	assert_ptr_equal(hash_map_find(&map, "key"), &first);
	assert_int_equal(hash_map_replace(&map, "key", &second), 0);
	assert_ptr_equal(hash_map_find(&map, "key"), &second);
	assert_int_equal(hash_map_destroy(&map), 0);
}

static void test_resize_failure_is_atomic(void **state)
{
	t_hash_map map = {0};
	t_hash_map_entry **old_buckets;
	void *(*old_alloc)(size_t);
	int value = 42;

	(void)state;
	assert_int_equal(hash_map_setup(&map, 2), 0);
	assert_int_equal(hash_map_add(&map, "key", &value), 0);
	old_buckets = map.map;
	old_alloc = g_types_settings.alloc;
	g_types_settings.alloc = fail_alloc;
	assert_int_equal(hash_map_resize(&map, 8), ENOMEM);
	g_types_settings.alloc = old_alloc;
	assert_ptr_equal(map.map, old_buckets);
	assert_int_equal(map.capacity, 2);
	assert_int_equal(map.length, 1);
	assert_ptr_equal(hash_map_find(&map, "key"), &value);
	assert_int_equal(hash_map_destroy(&map), 0);
}

static void test_resize_growth_and_remove(void **state)
{
	t_hash_map map = {0};
	int first = 1;
	int second = 2;

	(void)state;
	assert_int_equal(hash_map_setup(&map, 2), 0);
	assert_int_equal(hash_map_add(&map, "first", &first), 0);
	assert_int_equal(hash_map_add(&map, "second", &second), 0);
	assert_int_equal(hash_map_resize(&map, -1), 0);
	assert_true(map.capacity > 2);
	assert_ptr_equal(hash_map_find(&map, "first"), &first);
	assert_ptr_equal(hash_map_find(&map, "second"), &second);
	hash_map_remove(&map, "first");
	assert_int_equal(map.length, 1);
	assert_false(hash_map_contains(&map, "first"));
	assert_int_equal(hash_map_destroy(&map), 0);
}

int main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_setup_destroy_and_ownership),
		cmocka_unit_test(test_duplicate_key_and_replace),
		cmocka_unit_test(test_resize_failure_is_atomic),
		cmocka_unit_test(test_resize_growth_and_remove),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
