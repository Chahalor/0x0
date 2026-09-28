#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

#include "random.h"
#include "times.h"
#include "memory.h"

static void test_randint_extreme_bounds(void **state)
{
	int value;

	(void)state;
	srand(123);
	for (int i = 0; i < 100; i++)
	{
		value = randint(INT_MIN, INT_MAX);
		assert_true(value >= INT_MIN && value <= INT_MAX);
		value = randint(10, -10);
		assert_true(value >= -10 && value <= 10);
	}
	assert_int_equal(randint(INT_MIN, INT_MIN), INT_MIN);
	assert_int_equal(randint(INT_MAX, INT_MAX), INT_MAX);
}

static void test_rand_range(void **state)
{
	int *values;

	(void)state;
	values = rand_range(10, -3, 3);
	assert_non_null(values);
	for (size_t i = 0; i < 10; i++)
		assert_true(values[i] >= -3 && values[i] <= 3);
	assert_int_equal(values[10], 0);
	mem_free(values);
}

static void test_time_convert_checked(void **state)
{
	time_t result = 11;

	(void)state;
	assert_int_equal(time_convert_checked(3600, time_unity_sec,
		time_unity_hour, &result), 0);
	assert_int_equal(result, 1);
	assert_int_equal(time_convert_checked(0, time_unity_sec,
		time_unity_hour, &result), 0);
	assert_int_equal(result, 0);
	result = 11;
	assert_int_equal(time_convert_checked(1, 255,
		time_unity_sec, &result), EINVAL);
	assert_int_equal(result, 11);
	assert_int_equal(time_convert_checked(1, time_unity_sec,
		time_unity_ns, NULL), EINVAL);
	assert_int_equal(time_convert_checked((time_t)INT64_MAX,
		time_unity_day, time_unity_ns, &result), EOVERFLOW);
	assert_int_equal(result, 11);
	assert_int_equal(time_convert_checked((time_t)INT64_MAX,
		time_unity_sec, time_unity_hour, &result), 0);
	assert_true(result > 0);
	errno = 0;
	assert_int_equal(time_convert(0, time_unity_sec, time_unity_hour), 0);
	assert_int_equal(errno, 0);
	errno = 0;
	assert_int_equal(time_convert(1, 255, time_unity_hour), 0);
	assert_int_equal(errno, EINVAL);
}

int main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_randint_extreme_bounds),
		cmocka_unit_test(test_rand_range),
		cmocka_unit_test(test_time_convert_checked),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
