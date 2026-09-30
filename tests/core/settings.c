#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "core.h"

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

struct allocation_stats
{
	size_t allocations;
	size_t releases;
};

static struct allocation_stats *active_stats;

static void *tracked_alloc(size_t size)
{
	struct allocation_stats *stats = active_stats;

	stats->allocations++;
	return malloc(size);
}

static void tracked_free(void *ptr)
{
	struct allocation_stats *stats = active_stats;

	stats->releases++;
	free(ptr);
}

static int default_seven(void *output)
{
	*(int *)output = 7;
	return 0;
}

static int default_failure(void *output)
{
	(void)output;
	return EIO;
}

static int reject_negative(const void *value)
{
	return *(const int *)value < 0 ? -SETTING_ERROR_UNKNOWN : 0;
}

static void test_legacy_settings(void **state)
{
	t_genesis_core_settings settings = core_settings_default();
	t_genesis_core_allocator allocator = core_allocator_system();
	const t_genesis_core_allocator *selected;

	(void)state;
	selected = core_settings_allocator(&settings);
	assert_non_null(selected);
	assert_non_null(selected->alloc);
	assert_int_equal(core_settings_set_allocator(NULL, &allocator), EINVAL);
	assert_int_equal(core_settings_set_allocator(&settings, NULL), EINVAL);
	allocator.alloc = NULL;
	assert_int_equal(core_settings_set_allocator(&settings, &allocator), EINVAL);
	assert_non_null(core_settings_allocator(&settings)->alloc);
	allocator = core_allocator_system();
	assert_int_equal(core_settings_set_allocator(&settings, &allocator), 0);
	errno = 0;
	assert_null(core_settings_allocator(NULL));
	assert_int_equal(errno, EINVAL);
}

static void test_manager_validation_and_defaults(void **state)
{
	t_genesis_settings settings;
	t_genesis_core_allocator allocator;
	t_genesis_core_buffer_settings buffer_settings;
	const t_genesis_setting_key bad_name = { .size = sizeof(int) };
	const t_genesis_setting_key bad_size = { .name = "bad" };
	int value = 1;

	(void)state;
	assert_int_equal(genesis_settings_init(NULL), EINVAL);
	assert_int_equal(genesis_settings_init(&settings), 0);
	assert_int_equal(genesis_settings_get(&settings,
		&genesis_allocator_setting, &allocator, sizeof(allocator)), 0);
	assert_int_equal(core_allocator_validate(&allocator), 0);
	assert_int_equal(genesis_settings_get(&settings,
		&genesis_buffer_setting, &buffer_settings, sizeof(buffer_settings)), 0);
	assert_int_equal(buffer_settings.initial_capacity, 64);
	assert_int_equal(buffer_settings.growth, BUFFER_GROWTH_GEOMETRIC);
	value = 17;
	assert_int_equal(genesis_settings_set(&settings,
		&genesis_allocator_setting, &value, sizeof(value)), EINVAL);
	assert_int_equal(genesis_settings_get(&settings,
		&genesis_allocator_setting, &value, sizeof(value)), EINVAL);
	assert_int_equal(value, 17);
	assert_int_equal(genesis_settings_set(&settings,
		&genesis_allocator_setting, &allocator,
		sizeof(allocator) - 1), EINVAL);
	assert_int_equal(genesis_settings_get(&settings,
		&genesis_allocator_setting, &allocator,
		sizeof(allocator) + 1), EINVAL);
	assert_int_equal(genesis_settings_set(NULL, &genesis_allocator_setting,
		&allocator, sizeof(allocator)), EINVAL);
	assert_int_equal(genesis_settings_set(&settings, NULL, &value, sizeof(value)), EINVAL);
	assert_int_equal(genesis_settings_set(&settings, &bad_name, &value, sizeof(value)), EINVAL);
	assert_int_equal(genesis_settings_set(&settings, &bad_size, &value, sizeof(value)), EINVAL);
	assert_int_equal(genesis_settings_set(&settings,
		&genesis_allocator_setting, NULL, 0), EINVAL);
	assert_int_equal(genesis_settings_get(NULL,
		&genesis_allocator_setting, &allocator, sizeof(allocator)), EINVAL);
	assert_int_equal(genesis_settings_get(&settings, NULL, &value, sizeof(value)), EINVAL);
	assert_int_equal(genesis_settings_get(&settings, &bad_name, &value, sizeof(value)), EINVAL);
	assert_int_equal(genesis_settings_get(&settings,
		&genesis_allocator_setting, NULL, 0), EINVAL);
	assert_int_equal(genesis_settings_reset(NULL,
		&genesis_allocator_setting), EINVAL);
	assert_int_equal(genesis_settings_reset(&settings, NULL), EINVAL);
	assert_int_equal(genesis_settings_reset(&settings, &bad_size), EINVAL);
	genesis_settings_deinit(&settings);
	genesis_settings_deinit(NULL);
}

static void test_set_get_reset_and_key_identity(void **state)
{
	t_genesis_settings settings;
	t_genesis_settings other;
	const t_genesis_setting_key key_a = {
		.name = "same", .size = sizeof(int),
		.defaults = default_seven, .validate = reject_negative
	};
	const t_genesis_setting_key key_b = {
		.name = "same", .size = sizeof(int)
	};
	const t_genesis_setting_key failed_default = {
		.name = "failed", .size = sizeof(int), .defaults = default_failure
	};
	int value;

	(void)state;
	assert_int_equal(genesis_settings_init(&settings), 0);
	assert_int_equal(genesis_settings_init(&other), 0);
	assert_int_equal(genesis_settings_get(&settings, &key_a, &value, sizeof(value)), 0);
	assert_int_equal(value, 7);
	assert_int_equal(genesis_settings_get(&settings, &key_b, &value, sizeof(value)),
		-SETTING_ERROR_NOT_CONFIGURED);
	assert_int_equal(genesis_settings_get(&settings, &failed_default,
		&value, sizeof(value)), EIO);
	value = 12;
	assert_int_equal(genesis_settings_set(&settings, &key_a, &value, sizeof(value)), 0);
	value = 21;
	assert_int_equal(genesis_settings_set(&settings, &key_b, &value, sizeof(value)), 0);
	value = 0;
	assert_int_equal(genesis_settings_get(&settings, &key_a, &value, sizeof(value)), 0);
	assert_int_equal(value, 12);
	assert_int_equal(genesis_settings_get(&settings, &key_b, &value, sizeof(value)), 0);
	assert_int_equal(value, 21);
	value = -1;
	assert_int_equal(genesis_settings_set(&settings, &key_a, &value, sizeof(value)),
		-SETTING_ERROR_UNKNOWN);
	assert_int_equal(genesis_settings_get(&settings, &key_a, &value, sizeof(value)), 0);
	assert_int_equal(value, 12);
	assert_int_equal(genesis_settings_get(&other, &key_a, &value, sizeof(value)), 0);
	assert_int_equal(value, 7);
	assert_int_equal(genesis_settings_reset(&settings, &key_b), 0);
	assert_int_equal(genesis_settings_get(&settings, &key_b, &value, sizeof(value)),
		-SETTING_ERROR_NOT_CONFIGURED);
	assert_int_equal(genesis_settings_reset(&settings, &key_a), 0);
	assert_int_equal(genesis_settings_reset(&settings, &key_a), 0);
	assert_int_equal(genesis_settings_get(&settings, &key_a, &value, sizeof(value)), 0);
	assert_int_equal(value, 7);
	genesis_settings_deinit(&settings);
	genesis_settings_deinit(&other);
}

static void test_invalid_module_settings_are_atomic(void **state)
{
	t_genesis_settings settings;
	t_genesis_core_allocator allocator = core_allocator_system();
	t_genesis_core_allocator selected;
	t_genesis_core_buffer_settings buffer_settings = {
		.initial_capacity = 8, .growth = BUFFER_GROWTH_EXACT
	};
	t_genesis_core_buffer_settings selected_buffer;

	(void)state;
	assert_int_equal(genesis_settings_init(&settings), 0);
	assert_int_equal(genesis_settings_set(&settings,
		&genesis_allocator_setting, &allocator, sizeof(allocator)), 0);
	allocator.alloc = NULL;
	assert_int_equal(genesis_settings_set(&settings,
		&genesis_allocator_setting, &allocator, sizeof(allocator)), EINVAL);
	assert_int_equal(genesis_settings_get(&settings,
		&genesis_allocator_setting, &selected, sizeof(selected)), 0);
	assert_non_null(selected.alloc);
	assert_int_equal(genesis_settings_set(&settings,
		&genesis_buffer_setting, &buffer_settings, sizeof(buffer_settings)), 0);
	buffer_settings.initial_capacity = 0;
	assert_int_equal(genesis_settings_set(&settings,
		&genesis_buffer_setting, &buffer_settings, sizeof(buffer_settings)), EINVAL);
	buffer_settings.initial_capacity = 8;
	buffer_settings.growth = (t_genesis_core_buffer_growth)99;
	assert_int_equal(genesis_settings_set(&settings,
		&genesis_buffer_setting, &buffer_settings, sizeof(buffer_settings)), EINVAL);
	assert_int_equal(genesis_settings_get(&settings,
		&genesis_buffer_setting, &selected_buffer, sizeof(selected_buffer)), 0);
	assert_int_equal(selected_buffer.initial_capacity, 8);
	assert_int_equal(selected_buffer.growth, BUFFER_GROWTH_EXACT);
	genesis_settings_deinit(&settings);
}

/* Regression: resetting the head, middle and tail preserves remaining keys. */
static void test_reset_links_and_allocator_snapshot(void **state)
{
	struct allocation_stats stats = {0};
	t_genesis_core_allocator allocator = {
		.alloc = tracked_alloc, .free = tracked_free
	};
	t_genesis_settings settings;
	const t_genesis_setting_key a = { .name = "a", .size = sizeof(int) };
	const t_genesis_setting_key b = { .name = "b", .size = sizeof(int) };
	const t_genesis_setting_key c = { .name = "c", .size = sizeof(int) };
	const t_genesis_setting_key d = { .name = "d", .size = sizeof(int) };
	int value = 5;

	(void)state;
	active_stats = &stats;
	assert_int_equal(genesis_settings_init(&settings), 0);
	assert_int_equal(genesis_settings_set(&settings,
		&genesis_allocator_setting, &allocator, sizeof(allocator)), 0);
	assert_int_equal(genesis_settings_set(&settings, &a, &value, sizeof(value)), 0);
	assert_int_equal(genesis_settings_set(&settings, &b, &value, sizeof(value)), 0);
	assert_int_equal(genesis_settings_set(&settings, &c, &value, sizeof(value)), 0);
	assert_int_equal(genesis_settings_set(&settings, &d, &value, sizeof(value)), 0);
	assert_int_equal(genesis_settings_reset(&settings, &d), 0);
	assert_int_equal(genesis_settings_reset(&settings, &b), 0);
	assert_int_equal(genesis_settings_reset(&settings, &a), 0);
	assert_int_equal(genesis_settings_get(&settings, &c, &value, sizeof(value)), 0);
	assert_int_equal(value, 5);
	assert_int_equal(genesis_settings_reset(&settings,
		&genesis_allocator_setting), 0);
	genesis_settings_deinit(&settings);
	assert_int_equal(stats.allocations, 4);
	assert_int_equal(stats.releases, 4);
}

static void test_size_overflow(void **state)
{
	t_genesis_settings settings;
	const t_genesis_setting_key key = {
		.name = "huge", .size = SIZE_MAX
	};
	int value = 1;

	(void)state;
	assert_int_equal(genesis_settings_init(&settings), 0);
	/* Overflow is rejected before the deliberately impossible size is read. */
	assert_int_equal(genesis_settings_set(&settings, &key, &value, SIZE_MAX),
		EOVERFLOW);
	genesis_settings_deinit(&settings);
}

static void test_applied_global_default_allocator(void **state)
{
	struct allocation_stats stats = {0};
	t_genesis_core_allocator allocator = {
		.alloc = tracked_alloc, .free = tracked_free
	};
	t_genesis_core_allocator selected;
	t_genesis_settings local;
	t_genesis_core_settings legacy;
	t_genesis_core_buffer *buffer;

	(void)state;
	active_stats = &stats;
	assert_non_null(genesis_global_settings.apply);
	selected = genesis_default_allocator();
	assert_ptr_equal(selected.alloc, core_allocator_system().alloc);
	genesis_global_settings.allocator = allocator;
	selected = genesis_default_allocator();
	assert_ptr_equal(selected.alloc, core_allocator_system().alloc);
	assert_int_equal(genesis_global_settings.apply(), 0);
	selected = genesis_default_allocator();
	assert_ptr_equal(selected.alloc, tracked_alloc);
	legacy = core_settings_default();
	assert_ptr_equal(legacy.allocator.alloc, tracked_alloc);
	assert_int_equal(genesis_settings_init(&local), 0);
	assert_int_equal(genesis_settings_get(&local, &genesis_allocator_setting,
		&selected, sizeof(selected)), 0);
	assert_ptr_equal(selected.alloc, tracked_alloc);
	buffer = core_buffer_new(1);
	assert_non_null(buffer);
	core_buffer_free(buffer);
	assert_true(stats.allocations > 0);
	assert_int_equal(stats.allocations, stats.releases);
	genesis_settings_deinit(&local);
	genesis_global_settings.allocator.alloc = NULL;
	assert_int_equal(genesis_global_settings_apply(), EINVAL);
	selected = genesis_default_allocator();
	assert_ptr_equal(selected.alloc, tracked_alloc);
	genesis_global_settings.allocator =
		(t_genesis_core_allocator){0};
	assert_int_equal(genesis_global_settings.apply(), 0);
	selected = genesis_default_allocator();
	assert_ptr_equal(selected.alloc, core_allocator_system().alloc);
}

int main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_legacy_settings),
		cmocka_unit_test(test_manager_validation_and_defaults),
		cmocka_unit_test(test_set_get_reset_and_key_identity),
		cmocka_unit_test(test_invalid_module_settings_are_atomic),
		cmocka_unit_test(test_reset_links_and_allocator_snapshot),
		cmocka_unit_test(test_size_overflow),
		cmocka_unit_test(test_applied_global_default_allocator)
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
