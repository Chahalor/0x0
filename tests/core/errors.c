#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "core.h"

#include <errno.h>
#include <limits.h>
#include <string.h>

static const char *custom_error_message(int code)
{
	static const char *const messages[] = {
		[0] = "Custom module success",
		[1] = "Custom module error"
	};

	return messages[code == -1];
}

static void test_module_error_messages(void **state)
{
	(void)state;
	assert_string_equal(core_error_strerror(0), "Success");
	assert_string_equal(core_error_strerror(EINVAL), strerror(EINVAL));
	assert_string_equal(core_error_strerror(-CORE_ERROR_UNKNOWN),
		"Unknown core error");
	assert_string_equal(core_error_strerror(INT_MIN),
		"Unknown core error");
	assert_string_equal(core_setting_strerror(0), "Success");
	assert_string_equal(core_setting_strerror(ENOMEM), strerror(ENOMEM));
	assert_string_equal(core_setting_strerror(
		-SETTING_ERROR_NOT_CONFIGURED), "Setting is not configured");
	assert_string_equal(core_setting_strerror(-999),
		"Unknown setting error");
	assert_string_equal(core_allocator_strerror(0), "Success");
	assert_string_equal(core_allocator_strerror(EINVAL), strerror(EINVAL));
	assert_string_equal(core_allocator_strerror(-ALLOCATOR_ERROR_UNKNOWN),
		"Unknown allocator error");
	assert_string_equal(core_allocator_strerror(-999),
		"Unknown allocator error");
	assert_string_equal(core_buffer_strerror(0), "Success");
	assert_string_equal(core_buffer_strerror(ENOSPC), strerror(ENOSPC));
	assert_string_equal(core_buffer_strerror(
		-BUFFER_ERROR_SOURCE_OUT_OF_RANGE),
		"Source range is outside the buffer contents");
	assert_string_equal(core_buffer_strerror(INT_MIN),
		"Unknown buffer error");
}

static void test_central_dispatch(void **state)
{
	const t_genesis_error_module custom = {
		.name = "custom", .describe = custom_error_message
	};
	const t_genesis_error_module incomplete = { .name = "incomplete" };
	const t_genesis_error_module unnamed = {
		.describe = custom_error_message
	};

	(void)state;
	assert_string_equal(genesis_strerror(&genesis_core_error_module,
		-CORE_ERROR_UNKNOWN), "Unknown core error");
	assert_string_equal(genesis_strerror(&genesis_setting_error_module,
		-SETTING_ERROR_NOT_CONFIGURED), "Setting is not configured");
	assert_string_equal(genesis_strerror(&genesis_allocator_error_module,
		-ALLOCATOR_ERROR_UNKNOWN), "Unknown allocator error");
	assert_string_equal(genesis_strerror(&genesis_buffer_error_module,
		-BUFFER_ERROR_SOURCE_OUT_OF_RANGE),
		"Source range is outside the buffer contents");
	assert_string_equal(genesis_strerror(&genesis_buffer_error_module, EINVAL),
		strerror(EINVAL));
	assert_string_equal(genesis_strerror(&custom, -1),
		"Custom module error");
	assert_string_equal(genesis_strerror(NULL, 0),
		"Unknown Genesis module");
	assert_string_equal(genesis_strerror(&incomplete, 0),
		"Unknown Genesis module");
	assert_string_equal(genesis_strerror(&unnamed, 0),
		"Unknown Genesis module");
}

int main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_module_error_messages),
		cmocka_unit_test(test_central_dispatch)
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
