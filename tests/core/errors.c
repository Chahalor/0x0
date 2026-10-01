#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <errno.h>
#include <string.h>
#include "core/error/error.h"
#include "core/buffer/error.h"

static void test_error_dispatch(void **state)
{
	(void)state;
	assert_string_equal(core_error_strerror(0), "Success");
	assert_string_equal(core_buffer_strerror(EINVAL), strerror(EINVAL));
	assert_non_null(genesis_strerror(&genesis_buffer_error_module,
		-BUFFER_ERROR_SOURCE_OUT_OF_RANGE));
	assert_string_equal(genesis_strerror(NULL, EINVAL),
		"Unknown Genesis module");
}

int main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_error_dispatch)
	};

	return (cmocka_run_group_tests(tests, NULL, NULL));
}
