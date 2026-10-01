#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "types/uuid/uuid.h"
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

static uint64_t uuid_milliseconds(const UUID *uuid)
{
	uint64_t result = 0;
	size_t i;

	for (i = 0; i < 6; ++i)
		result = (result << 8) | uuid->bytes[i];
	return (result);
}

static uint64_t current_milliseconds(void)
{
	struct timespec now;

	assert_int_equal(clock_gettime(CLOCK_REALTIME, &now), 0);
	return ((uint64_t)now.tv_sec * 1000u
		+ (uint64_t)now.tv_nsec / 1000000u);
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
	assert_true(uuid_milliseconds(uuid) >= before);
	assert_true(uuid_milliseconds(uuid) <= after);
	assert_int_equal(genesis_uuid_version(uuid), 7);
	assert_int_equal(uuid->bytes[8] & 0xc0, 0x80);
	text = genesis_uuid_stringify(uuid);
	assert_non_null(text);
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
	UUID before;
	char *text;

	(void)state;
	uuid = genesis_uuid_read(upper);
	assert_non_null(uuid);
	assert_int_equal(genesis_uuid_version(uuid), 7);
	text = genesis_uuid_stringify(uuid);
	assert_string_equal(text, lower);
	mem_free(text);
	before = *uuid;
	assert_int_equal(genesis_uuid_read_set(uuid,
		"018f0e12-3456-7abc-8def-0123456789ag"),
		-UUID_ERROR_INVALID_FORMAT);
	assert_memory_equal(uuid, &before, sizeof(before));
	assert_int_equal(genesis_uuid_read_set(uuid, "bad"),
		-UUID_ERROR_INVALID_FORMAT);
	assert_memory_equal(uuid, &before, sizeof(before));
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
	UUID uuid = UUID_MAX;
	UUID original = uuid;

	(void)state;
	genesis_uuid_setting.getrandom = failed_random;
	assert_int_equal(genesis_uuid_setting.apply(), 0);
	assert_int_equal(genesis_uuid_set(&uuid), EIO);
	assert_memory_equal(&uuid, &original, sizeof(uuid));
	errno = 0;
	assert_null(genesis_uuid());
	assert_int_equal(errno, EIO);
	genesis_uuid_setting.default_version = 4;
	assert_int_equal(genesis_uuid_setting.apply(), EINVAL);
	assert_int_equal(genesis_uuid_set(&uuid), EIO);
	genesis_uuid_setting.default_version = 7;
	genesis_uuid_setting.getrandom = NULL;
	assert_int_equal(genesis_uuid_setting.apply(), EINVAL);
	assert_int_equal(genesis_uuid_set(&uuid), EIO);
	genesis_uuid_setting.getrandom = empty_random;
	assert_int_equal(genesis_uuid_setting.apply(), 0);
	assert_int_equal(genesis_uuid_set(&uuid), EIO);
	assert_memory_equal(&uuid, &original, sizeof(uuid));
	genesis_uuid_setting = saved;
	assert_int_equal(genesis_uuid_setting.apply(), 0);
	errno = 0;
	assert_null(genesis_uuid_generate(4));
	assert_int_equal(errno, EINVAL);
	assert_int_equal(genesis_uuid_set(NULL), EINVAL);
	genesis_uuid_free(NULL);
}

static void test_comparison_and_null(void **state)
{
	UUID low = UUID_NULL;
	UUID high = UUID_MAX;

	(void)state;
	assert_int_equal(genesis_uuid_cmp(&low, &low), 0);
	assert_true(genesis_uuid_cmp(&low, &high) < 0);
	assert_true(genesis_uuid_cmp(&high, &low) > 0);
	assert_int_equal(genesis_uuid_cmp(NULL, NULL), 0);
	assert_true(genesis_uuid_cmp(NULL, &low) < 0);
	assert_int_equal(genesis_uuid_version(NULL), INT_MAX);
	assert_int_equal(genesis_uuid_read_set(NULL, "bad"), EINVAL);
	errno = 0;
	assert_null(genesis_uuid_read(NULL));
	assert_int_equal(errno, EINVAL);
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
		cmocka_unit_test(test_parse_roundtrip_and_invalid),
		cmocka_unit_test(test_failures_and_atomic_settings),
		cmocka_unit_test(test_comparison_and_null),
		cmocka_unit_test(test_allocation_failure),
		cmocka_unit_test(test_error_messages)
	};

	return (cmocka_run_group_tests(tests, NULL, NULL));
}
