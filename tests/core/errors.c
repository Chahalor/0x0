#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include "core.h"

#include <errno.h>
#include <limits.h>
#include <string.h>

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
	(void)state;
	assert_string_equal(genesis_strerror(GENESIS_MODULE_ERROR,
		-CORE_ERROR_UNKNOWN), "Unknown core error");
	assert_string_equal(genesis_strerror(GENESIS_MODULE_SETTING,
		-SETTING_ERROR_NOT_CONFIGURED), "Setting is not configured");
	assert_string_equal(genesis_strerror(GENESIS_MODULE_ALLOCATOR,
		-ALLOCATOR_ERROR_UNKNOWN), "Unknown allocator error");
	assert_string_equal(genesis_strerror(GENESIS_MODULE_BUFFER,
		-BUFFER_ERROR_SOURCE_OUT_OF_RANGE),
		"Source range is outside the buffer contents");
	assert_string_equal(genesis_strerror(GENESIS_MODULE_BUFFER, EINVAL),
		strerror(EINVAL));
	assert_string_equal(genesis_strerror((t_genesis_core_module)-1, 0),
		"Unknown Genesis module");
	assert_string_equal(genesis_strerror((t_genesis_core_module)99, 0),
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
