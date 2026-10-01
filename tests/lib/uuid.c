#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/random.h>
#include <time.h>
#include "types/uuid.h"
#include "types/uuid/setting.h"
#include "memory.h"
#include "core/setting/setting.h"
#include "core/error/error.h"

static size_t random_calls;

static ssize_t short_random(void *output, size_t size, unsigned int flags)
{
	(void)flags;
	random_calls++;
	if (size > 3)
		size = 3;
	memset(output, 0xaa, size);
	return ((ssize_t)size);
}

static ssize_t failed_random(void *output, size_t size, unsigned int flags)
{
	(void)output;
	(void)size;
	(void)flags;
	errno = EIO;
	return (-1);
}

static ssize_t empty_random(void *output, size_t size, unsigned int flags)
{
	(void)output;
	(void)size;
	(void)flags;
	return (0);
}

static void *failed_alloc(size_t size)
{
	(void)size;
	return (NULL);
}

static uint64_t uuid_milliseconds(const char *text)
{
	char timestamp[13];

	memcpy(timestamp, text, 8);
	memcpy(timestamp + 8, text + 9, 4);
	timestamp[12] = '\0';
	return (strtoull(timestamp, NULL, 16));
}

static uint64_t current_milliseconds(void)
{
	struct timespec now;

	assert_int_equal(clock_gettime(CLOCK_REALTIME, &now), 0);
	return ((uint64_t)now.tv_sec * 1000u
		+ (uint64_t)now.tv_nsec / 1000000u);
}

static int compare_uuid_text(const void *left, const void *right)
{
	return (strcmp(left, right));
}

static void test_generated_uuids_have_no_collisions(void **state)
{
	const size_t count = 10000;
	t_genesis_uuid_setting saved = genesis_uuid_setting;
	char (*values)[UUID_STRING_SIZE];
	UUID *uuid;
	char *text;
	size_t i;

	(void)state;
	values = malloc(count * sizeof(*values));
	assert_non_null(values);
	genesis_uuid_setting.default_version = 7;
	genesis_uuid_setting.getrandom = getrandom;
	assert_int_equal(genesis_uuid_setting.apply(), 0);
	for (i = 0; i < count; ++i)
	{
		uuid = genesis_uuid();
		assert_non_null(uuid);
		text = genesis_uuid_stringify(uuid);
		assert_non_null(text);
		memcpy(values[i], text, UUID_STRING_SIZE);
		mem_free(text);
		genesis_uuid_free(uuid);
	}
	qsort(values, count, sizeof(*values), compare_uuid_text);
	for (i = 1; i < count; ++i)
		assert_true(strcmp(values[i - 1], values[i]) != 0);
	genesis_uuid_setting = saved;
	assert_int_equal(genesis_uuid_setting.apply(), 0);
	free(values);
}

static void test_generate_v7_and_short_random(void **state)
{
	t_genesis_uuid_setting saved = genesis_uuid_setting;
	uint64_t before;
	uint64_t after;
	UUID *uuid;
	char *text;

	(void)state;
	genesis_uuid_setting.getrandom = short_random;
	assert_int_equal(genesis_uuid_setting.apply(), 0);
	random_calls = 0;
	before = current_milliseconds();
	uuid = genesis_uuid();
	after = current_milliseconds();
	assert_non_null(uuid);
	assert_true(random_calls > 1);
	assert_int_equal(genesis_uuid_version(uuid), 7);
	text = genesis_uuid_stringify(uuid);
	assert_non_null(text);
	assert_true(uuid_milliseconds(text) >= before);
	assert_true(uuid_milliseconds(text) <= after);
	assert_true(text[19] == '8' || text[19] == '9'
		|| text[19] == 'a' || text[19] == 'b');
	assert_int_equal(strlen(text), 36);
	mem_free(text);
	genesis_uuid_free(uuid);
	genesis_uuid_setting = saved;
	assert_int_equal(genesis_uuid_settings_apply(), 0);
}

static void test_parse_roundtrip_and_invalid(void **state)
{
	const char *upper = "018F0E12-3456-7ABC-8DEF-0123456789AB";
	const char *lower = "018f0e12-3456-7abc-8def-0123456789ab";
	UUID *uuid;
	char *text;

	(void)state;
	uuid = genesis_uuid_read(upper);
	assert_non_null(uuid);
	assert_int_equal(genesis_uuid_version(uuid), 7);
	text = genesis_uuid_stringify(uuid);
	assert_string_equal(text, lower);
	mem_free(text);
	assert_int_equal(genesis_uuid_read_set(uuid,
		"018f0e12-3456-7abc-8def-0123456789ag"),
		-UUID_ERROR_INVALID_FORMAT);
	text = genesis_uuid_stringify(uuid);
	assert_string_equal(text, lower);
	mem_free(text);
	assert_int_equal(genesis_uuid_read_set(uuid, "bad"),
		-UUID_ERROR_INVALID_FORMAT);
	text = genesis_uuid_stringify(uuid);
	assert_string_equal(text, lower);
	mem_free(text);
	errno = 0;
	assert_null(genesis_uuid_read("bad"));
	assert_int_equal(errno, EINVAL);
	genesis_uuid_free(uuid);
	text = genesis_uuid_stringify(NULL);
	assert_string_equal(text, "00000000-0000-0000-0000-000000000000");
	mem_free(text);
}

static void test_failures_and_atomic_settings(void **state)
{
	t_genesis_uuid_setting saved = genesis_uuid_setting;
	const char *max = "ffffffff-ffff-ffff-ffff-ffffffffffff";
	UUID *uuid;
	char *text;

	(void)state;
	uuid = genesis_uuid_read(max);
	assert_non_null(uuid);
	genesis_uuid_setting.getrandom = failed_random;
	assert_int_equal(genesis_uuid_setting.apply(), 0);
	assert_int_equal(genesis_uuid_set(uuid), EIO);
	text = genesis_uuid_stringify(uuid);
	assert_string_equal(text, max);
	mem_free(text);
	errno = 0;
	assert_null(genesis_uuid());
	assert_int_equal(errno, EIO);
	genesis_uuid_setting.default_version = 4;
	assert_int_equal(genesis_uuid_setting.apply(), EINVAL);
	assert_int_equal(genesis_uuid_set(uuid), EIO);
	genesis_uuid_setting.default_version = 7;
	genesis_uuid_setting.getrandom = NULL;
	assert_int_equal(genesis_uuid_setting.apply(), EINVAL);
	assert_int_equal(genesis_uuid_set(uuid), EIO);
	genesis_uuid_setting.getrandom = empty_random;
	assert_int_equal(genesis_uuid_setting.apply(), 0);
	assert_int_equal(genesis_uuid_set(uuid), EIO);
	text = genesis_uuid_stringify(uuid);
	assert_string_equal(text, max);
	mem_free(text);
	genesis_uuid_setting = saved;
	assert_int_equal(genesis_uuid_setting.apply(), 0);
	genesis_uuid_free(uuid);
	errno = 0;
	assert_null(genesis_uuid_generate(4));
	assert_int_equal(errno, EINVAL);
	assert_int_equal(genesis_uuid_set(NULL), EINVAL);
	genesis_uuid_free(NULL);
}

static void test_comparison_and_null(void **state)
{
	UUID *low;
	UUID *high;

	(void)state;
	low = genesis_uuid_read("00000000-0000-0000-0000-000000000000");
	high = genesis_uuid_read("ffffffff-ffff-ffff-ffff-ffffffffffff");
	assert_non_null(low);
	assert_non_null(high);
	assert_int_equal(genesis_uuid_cmp(low, low), 0);
	assert_true(genesis_uuid_cmp(low, high) < 0);
	assert_true(genesis_uuid_cmp(high, low) > 0);
	assert_int_equal(genesis_uuid_cmp(NULL, NULL), 0);
	assert_true(genesis_uuid_cmp(NULL, low) < 0);
	assert_int_equal(genesis_uuid_version(NULL), INT_MAX);
	assert_int_equal(genesis_uuid_read_set(NULL, "bad"), EINVAL);
	errno = 0;
	assert_null(genesis_uuid_read(NULL));
	assert_int_equal(errno, EINVAL);
	genesis_uuid_free(low);
	genesis_uuid_free(high);
}

static void test_allocation_failure(void **state)
{
	void *(*old_alloc)(size_t) = G_SETTINGS.allocator.alloc;

	(void)state;
	mem_free_all();
	G_SETTINGS.allocator.alloc = failed_alloc;
	errno = 0;
	assert_null(genesis_uuid());
	assert_int_equal(errno, ENOMEM);
	errno = 0;
	assert_null(genesis_uuid_stringify(NULL));
	assert_int_equal(errno, ENOMEM);
	G_SETTINGS.allocator.alloc = old_alloc;
	mem_free_all();
}

static void test_error_messages(void **state)
{
	(void)state;
	assert_string_equal(genesis_uuid_strerror(0), "Success");
	assert_string_equal(genesis_uuid_strerror(EINVAL), strerror(EINVAL));
	assert_string_equal(genesis_uuid_strerror(-UUID_ERROR_INVALID_FORMAT),
		"Invalid UUID format");
	assert_string_equal(genesis_uuid_strerror(-UUID_ERROR_UNSUPPORTED_VERSION),
		"Unsupported UUID version");
	assert_string_equal(genesis_uuid_strerror(-9999), "Unknown UUID error");
	assert_string_equal(genesis_strerror(&genesis_uuid_error_module,
		-UUID_ERROR_INVALID_FORMAT), "Invalid UUID format");
}

int main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_generate_v7_and_short_random),
		cmocka_unit_test(test_generated_uuids_have_no_collisions),
		cmocka_unit_test(test_parse_roundtrip_and_invalid),
		cmocka_unit_test(test_failures_and_atomic_settings),
		cmocka_unit_test(test_comparison_and_null),
		cmocka_unit_test(test_allocation_failure),
		cmocka_unit_test(test_error_messages)
	};

	return (cmocka_run_group_tests(tests, NULL, NULL));
}
