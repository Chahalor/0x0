/**
 * @file format.c
 * @brief CMocka contract and regression tests for the format module.
 */

/* ----| Headers    |----- */
	/* Standard */
# include <stdarg.h>
# include <stddef.h>
# include <setjmp.h>
# include <errno.h>
# include <stdint.h>
# include <stdlib.h>
# include <string.h>
# include <stdio.h>
# include <inttypes.h>
# include <sys/types.h>
# include <wchar.h>

	/* External */
# include <cmocka.h>
# include "interface/format.h"
# include "core/setting/setting.h"
# include "interface/format/setting.h"
# include "interface/format/error.h"
# include "core/error/error.h"

/* ----| Prototypes |----- */
static void	count_free(
	void	*ptr
);
static void	restore_format_defaults(void);

/* ----| Internals  |----- */
static char	*custom(
	const void					   *value,
	const char					   *options
)
{
	const char *v	= value;
	char	   *out = mem_alloc(strlen(v) + strlen(options) + 2);
	if (!out)
		return (NULL);
	strcpy(out, v);
	strcat(out, ":");
	strcat(out, options);
	return (out);
}

static void	basics(
	void **state
)
{
	char *s;
	(void)state;
	errno = 0;
	assert_null(mem_alloc(0));
	assert_int_equal(errno, EINVAL);
	errno = 0;
	assert_null(mem_alloc(SIZE_MAX));
	assert_int_equal(errno, EOVERFLOW);
	s = mem_alloc(4);
	assert_non_null(s);
	memcpy(s, "yes", 4);
	assert_string_equal(s, "yes");
	mem_free(s);
	s = genesis_format("%s %d %#x %04u %%", "hi", -12, 42u, 7u);
	assert_non_null(s);
	assert_string_equal(s, "hi -12 0x2a 0007 %");
	mem_free(s);
	s = genesis_format("%+8.3d/%-6.3s", 12, "hello");
	assert_string_equal(s, "    +012/hel   ");
	mem_free(s);
}
static void	fixed(
	void **state
)
{
	char out[4];
	(void)state;
	assert_int_equal(genesis_sformat(out, sizeof out, "%s", "hello"), 5);
	assert_string_equal(out, "hel");
	assert_int_equal(genesis_sformat(NULL, 0, "%d", 123), 3);
	assert_int_equal(genesis_sformat(out, sizeof out, ""), 0);
	assert_string_equal(out, "");
}
static void	tags(
	void **state
)
{
	char *s;
	(void)state;
	assert_int_equal(genesis_format_set("ITEM", custom), 0);
	s = genesis_format("[%{ITEM:one:two}]", (void *)"value");
	assert_string_equal(s, "[value:one:two]");
	mem_free(s);
	assert_int_equal(genesis_format_rm("ITEM"), 0);
	assert_int_equal(genesis_format_rm("ITEM"), -FORMAT_ERROR_UNKNOWN_TAG);
	errno = 0;
	assert_null(genesis_format("%{ITEM}", (void *)"value"));
	assert_int_equal(errno, EINVAL);
}

static void	test_custom_options_expand_in_argument_order(
	void **state
)
{
	char	*result;
	char	output[32];
	int		option_count = -1;

	(void)state;
	assert_int_equal(genesis_format_set("OPT_ORDER", custom), 0);
	result = genesis_format("%{OPT_ORDER:%s-%d}:%s", "wide", 7,
		(void *)"item", "tail");
	assert_non_null(result);
	assert_string_equal(result, "item:wide-7:tail");
	mem_free(result);
	assert_int_equal(genesis_sformat(output, sizeof(output),
		"%{OPT_ORDER:%s}", "short", (void *)"value"), 11);
	assert_string_equal(output, "value:short");
	result = genesis_format("[%{OPT_ORDER:%%/%*d}]", 3, 5,
		(void *)"value");
	assert_non_null(result);
	assert_string_equal(result, "[value:%/  5]");
	mem_free(result);
	result = genesis_format("%{OPT_ORDER:pre%n%s}", &option_count,
		"late", (void *)"value");
	assert_non_null(result);
	assert_int_equal(option_count, 3);
	assert_string_equal(result, "value:prelate");
	mem_free(result);
	assert_int_equal(genesis_format_rm("OPT_ORDER"), 0);
}
static void	test_first_tag_after_empty_registry(void **state)
{
	char *result;

	(void)state;
	assert_int_equal(genesis_format_set("FIRST", custom), 0);
	result = genesis_format("%{FIRST:ok}", (void *)"value");
	assert_string_equal(result, "value:ok");
	mem_free(result);
	assert_int_equal(genesis_format_rm("FIRST"), 0);
	assert_int_equal(genesis_format_set("FIRST", custom), 0);
	assert_int_equal(genesis_format_rm("FIRST"), 0);
}
static void	errors(
	void **state
)
{
	char out[4] = "abc";
	(void)state;
	assert_int_equal(genesis_format_set("bad:name", custom), EINVAL);
	assert_int_equal(genesis_format_set(NULL, custom), EINVAL);
	assert_int_equal(genesis_sformat(out, sizeof out, "%q", 1), SIZE_MAX);
	assert_string_equal(out, "");
	assert_null(genesis_format(NULL));
	assert_int_equal(errno, EINVAL);
}
static void	floats_and_n(
	void **state
)
{
	char *s;
	int	  n = -1;
	(void)state;
	s = genesis_format("%.2f/%g%n", 1.25, 2.0, &n);
	assert_string_equal(s, "1.25/2");
	assert_int_equal(n, 6);
	mem_free(s);
}

static void	standard_boundaries(
	void **state
)
{
	char  expected[128];
	char  actual[128];
	char *s;
	(void)state;
	snprintf(expected, sizeof expected, "[%*.*d][%#08x][%#.0o]", -8, 4, -12,
			 42u, 0u);
	assert_int_equal(genesis_sformat(actual, sizeof actual,
									 "[%*.*d][%#08x][%#.0o]", -8, 4, -12, 42u,
									 0u),
					 strlen(expected));
	assert_string_equal(actual, expected);
	snprintf(expected, sizeof expected, "[%+08d][%.0u][%-5c]", 42, 0u, 'Q');
	assert_int_equal(genesis_sformat(actual, sizeof actual,
									 "[%+08d][%.0u][%-5c]", 42, 0u, 'Q'),
					 strlen(expected));
	assert_string_equal(actual, expected);
	s = genesis_format("%lld/%zu/%p", (long long)-9223372036854775807LL,
					   (size_t)123, (void *)actual);
	assert_non_null(s);
	mem_free(s);
}

static void	malformed(
	void **state
)
{
	char result[8] = "old";
	(void)state;
	assert_int_equal(genesis_sformat(result, sizeof result, "%Ls", "a"),
					 SIZE_MAX);
	assert_string_equal(result, "");
	assert_int_equal(genesis_sformat(result, sizeof result, "%n", (int *)NULL),
					 SIZE_MAX);
	assert_string_equal(result, "");
	assert_int_equal(
		genesis_sformat(result, sizeof result, "%{BROKEN", (void *)NULL),
		SIZE_MAX);
	assert_string_equal(result, "");
}

typedef struct s_counting_allocator
{
	size_t allocations;
	size_t frees;
} t_counting_allocator;
static t_counting_allocator *active_counter;
static void	*count_alloc(
	size_t size
)
{
	t_counting_allocator *counter = active_counter;
	++counter->allocations;
	return (malloc(size));
}
static void	*count_realloc(
	void *ptr,
	size_t new_size
)
{
	t_counting_allocator *counter = active_counter;
	if (!ptr)
		++counter->allocations;
	return (realloc(ptr, new_size));
}
static void	count_free(
	void	*ptr
)
{
	t_counting_allocator *counter = active_counter;
	++counter->frees;
	free(ptr);
}

static void	restore_format_defaults(void)
{
	genesis_format_settings.max_output = SIZE_MAX - 1;
	assert_int_equal(genesis_format_settings.apply(), 0);
}

static void	settings_contract(void **state)
{
	char *result;
	char fixed[8];

	(void)state;
	assert_non_null(genesis_format_settings.apply);
	genesis_format_settings.max_output = 12;
	/* Direct edits do not change the active configuration. */
	result = genesis_format("1234567890123");
	assert_string_equal(result, "1234567890123");
	mem_free(result);
	assert_int_equal(genesis_format_settings_apply(), 0);
	assert_int_equal(genesis_sformat(fixed, sizeof(fixed), "%s",
		"1234567890123"), SIZE_MAX);
	assert_int_equal(errno, EOVERFLOW);
	assert_string_equal(fixed, "");
	genesis_format_settings.max_output = SIZE_MAX;
	assert_int_equal(genesis_format_settings.apply(), EINVAL);
	result = genesis_format("within limit");
	assert_string_equal(result, "within limit");
	mem_free(result);
	restore_format_defaults();
}

static void	test_global_allocator_is_global(void **state)
{
	t_counting_allocator counter = {0};
	t_genesis_global_settings settings = {.allocator = {
		.alloc = count_alloc, .realloc = count_realloc, .free = count_free
	}};
	char *result;
	size_t allocations;

	(void)state;
	active_counter = &counter;
	G_SETTINGS = settings;
	assert_int_equal(genesis_format_set("GLOBAL", custom), 0);
	assert_true(counter.allocations > 0);
	allocations = counter.allocations;
	result = genesis_format("%{GLOBAL:x}", (void *)"value");
	assert_string_equal(result, "value:x");
	mem_free(result);
	assert_true(counter.allocations > allocations);
	mem_free_all();
	G_SETTINGS = (t_genesis_global_settings){
		.allocator = {.alloc = malloc, .free = free, .realloc = realloc}
	};
	assert_int_equal(genesis_format_rm("GLOBAL"), 0);
	assert_true(counter.frees > 0);
}

static void	registry_growth(
	void **state
)
{
	char  tag[32];
	char *result;
	int	  i;
	(void)state;
	for (i = 0; i < 128; ++i)
	{
		snprintf(tag, sizeof tag, "TAG_%d", i);
		assert_int_equal(genesis_format_set(tag, custom), 0);
	}
	result = genesis_format("%{TAG_127:ready}", (void *)"hash");
	assert_non_null(result);
	assert_string_equal(result, "hash:ready");
	mem_free(result);
	for (i = 0; i < 128; ++i)
	{
		snprintf(tag, sizeof tag, "TAG_%d", i);
		assert_int_equal(genesis_format_rm(tag), 0);
	}
}

static char	*call_vformat(
	const char *format,
	...
)
{
	va_list list;
	char   *result;
	va_start(list, format);
	result = genesis_vformat(format, list);
	va_end(list);
	return (result);
}

static size_t	call_vsformat(
	char *output,
	size_t size,
	const char *format,
	...
)
{
	va_list list;
	size_t	result;
	va_start(list, format);
	result = genesis_vsformat(output, size, format, list);
	va_end(list);
	return (result);
}

static void	test_va_forms_and_errors(
	void **state
)
{
	char  output[5];
	char *result;
	(void)state;
	result = call_vformat("%s:%d", "v", 9);
	assert_string_equal(result, "v:9");
	mem_free(result);
	assert_int_equal(genesis_format_set("VA_OPT", custom), 0);
	result = call_vformat("%{VA_OPT:%d}", 42, (void *)"value");
	assert_string_equal(result, "value:42");
	mem_free(result);
	assert_int_equal(call_vsformat(output, sizeof(output),
		"%{VA_OPT:%s}", "small", (void *)"v"), 7);
	assert_string_equal(output, "v:sm");
	assert_int_equal(genesis_format_rm("VA_OPT"), 0);
	assert_int_equal(call_vsformat(output, sizeof output, "%s", "abcdef"), 6);
	assert_string_equal(output, "abcd");
	assert_string_equal(genesis_format_strerror(0), "Success");
	assert_string_equal(genesis_strerror(&genesis_format_error_module,
										 -FORMAT_ERROR_UNKNOWN_TAG),
						"Unknown custom tag");
	assert_non_null(genesis_format_strerror(EINVAL));
	assert_string_equal(genesis_format_strerror(-9999), "Unknown format error");
	mem_free(NULL);
}

static char	*replacement(
	const void						*value,
	const char						*options
)
{
	char *result;
	(void)value;
	(void)options;
	result = mem_alloc(4);
	if (!result)
		return (NULL);
	memcpy(result, "new", 4);
	return (result);
}

static void	test_registry_replace_and_invalid(
	void **state
)
{
	char *result;
	(void)state;
	assert_int_equal(genesis_format_set("REPLACE", custom), 0);
	assert_int_equal(genesis_format_set("REPLACE", replacement), 0);
	result = genesis_format("%{REPLACE}", (void *)"old");
	assert_string_equal(result, "new");
	mem_free(result);
	assert_int_equal(genesis_format_rm("REPLACE"), 0);
	assert_int_equal(genesis_format_rm("bad:name"), EINVAL);
	assert_int_equal(genesis_format_rm("{bad"), EINVAL);
	assert_int_equal(genesis_format_set("{bad", custom), EINVAL);
	assert_int_equal(genesis_format_set("TAG", NULL), EINVAL);
}

static void	*fail_alloc(
	size_t size
)
{
	(void)size;
	return (NULL);
}

static void	test_allocator_failure(void **state)
{
	t_genesis_global_settings settings = {.allocator = {
		.alloc = fail_alloc, .free = count_free
	}};
	char *result;

	(void)state;
	G_SETTINGS = settings;
	assert_int_equal(genesis_format_set("FAIL_NEW", custom), ENOMEM);
	errno = 0;
	result = genesis_format("hello");
	assert_null(result);
	assert_int_equal(errno, ENOMEM);
	G_SETTINGS = (t_genesis_global_settings){
		.allocator = {.alloc = malloc, .free = free, .realloc = realloc}
	};
	assert_int_equal(genesis_format_rm("FAIL_NEW"),
		-FORMAT_ERROR_UNKNOWN_TAG);
}

static char	*callback_failure(
	const void					 *value,
	const char					 *options
)
{
	(void)value;
	(void)options;
	errno = EIO;
	return (NULL);
}

static void	test_callback_failure(
	void **state
)
{
	char output[8] = "old";
	(void)state;
	assert_int_equal(genesis_format_set("ERROR", callback_failure), 0);
	errno = 0;
	assert_null(genesis_format("%{ERROR}", (void *)"x"));
	assert_int_equal(errno, EIO);
	assert_int_equal(
		genesis_sformat(output, sizeof output, "%{ERROR}", (void *)"x"),
		SIZE_MAX);
	assert_string_equal(output, "");
	errno = 0;
	assert_null(genesis_format("%{ERROR:%s}", "expanded", (void *)"x"));
	assert_int_equal(errno, EIO);
	assert_int_equal(genesis_format_rm("ERROR"), 0);
}

static int	option_callback_calls;

static char	*count_option_callback(
	const void	*value,
	const char	*options
)
{
	char	*result;

	(void)value;
	(void)options;
	++option_callback_calls;
	result = mem_alloc(3);
	if (result)
		memcpy(result, "ok", 3);
	return (result);
}

static void	test_custom_option_errors_skip_callback(
	void **state
)
{
	char	output[16] = "old";
	t_genesis_global_settings saved_settings = G_SETTINGS;

	(void)state;
	option_callback_calls = 0;
	assert_int_equal(genesis_format_set("OPT_ERROR", count_option_callback), 0);
	errno = 0;
	assert_null(genesis_format("%{OPT_ERROR:%q}", (void *)"value"));
	assert_int_equal(errno, EINVAL);
	assert_int_equal(option_callback_calls, 0);
	errno = 0;
	assert_int_equal(genesis_sformat(output, sizeof(output),
		"%{OPT_ERROR:%s}", (char *)NULL, (void *)"value"), SIZE_MAX);
	assert_int_equal(errno, EINVAL);
	assert_string_equal(output, "");
	assert_int_equal(option_callback_calls, 0);
	genesis_format_settings.max_output = 3;
	assert_int_equal(genesis_format_settings.apply(), 0);
	errno = 0;
	assert_null(genesis_format("%{OPT_ERROR:%s}", "long",
		(void *)"value"));
	assert_int_equal(errno, EOVERFLOW);
	assert_int_equal(option_callback_calls, 0);
	restore_format_defaults();
	G_SETTINGS.allocator.alloc = fail_alloc;
	errno = 0;
	assert_null(genesis_format("%{OPT_ERROR:text}", (void *)"value"));
	assert_int_equal(errno, ENOMEM);
	assert_int_equal(option_callback_calls, 0);
	G_SETTINGS = saved_settings;
	assert_int_equal(genesis_format_rm("OPT_ERROR"), 0);
}

static void	test_standard_conversion_matrix(
	void **state
)
{
	char	expected[256];
	char	actual[256];
	wchar_t wide[] = L"wide";
	short	count  = -1;
	ssize_t	size_count = -1;
	(void)state;
	snprintf(expected, sizeof expected, "%hhd/%hu/%jd/%zu/%td/%llX", (int)-12,
			 (unsigned int)65530, (intmax_t)-123, (size_t)42, (ptrdiff_t)-7,
			 (unsigned long long)0xABCD);
	assert_int_equal(
		genesis_sformat(actual, sizeof actual, "%hhd/%hu/%jd/%zu/%td/%llX",
						(int)-12, (unsigned int)65530, (intmax_t)-123,
						(size_t)42, (ptrdiff_t)-7, (unsigned long long)0xABCD),
		strlen(expected));
	assert_string_equal(actual, expected);
	snprintf(expected, sizeof expected, "%.2Lf/%a/%E/%G/%ls/%lc",
			 (long double)1.25, 2.0, 3.0, 4.0, wide, (wint_t)L'Q');
	assert_int_equal(
		genesis_sformat(actual, sizeof actual, "%.2Lf/%a/%E/%G/%ls/%lc",
						(long double)1.25, 2.0, 3.0, 4.0, wide, (wint_t)L'Q'),
		strlen(expected));
	assert_string_equal(actual, expected);
	assert_int_equal(genesis_sformat(actual, sizeof actual, "abc%hn", &count),
					 3);
	assert_int_equal(count, 3);
	assert_string_equal(actual, "abc");
	assert_int_equal(genesis_sformat(actual, sizeof actual, "abc%zn",
			&size_count), 3);
	assert_int_equal(size_count, 3);
	assert_int_equal(genesis_sformat(actual, 1, "%s", "abc"), 3);
	assert_string_equal(actual, "");
}

static void	test_settings_defaults_and_zero(
	void **state
)
{
	char *result;

	(void)state;
	result = genesis_format("ok");
	assert_string_equal(result, "ok");
	mem_free(result);
	genesis_format_settings.max_output = 0;
	assert_int_equal(genesis_format_settings.apply(), 0);
	result = genesis_format("");
	assert_string_equal(result, "");
	mem_free(result);
	errno = 0;
	assert_null(genesis_format("x"));
	assert_int_equal(errno, EOVERFLOW);
	restore_format_defaults();
	result = genesis_format("x");
	assert_string_equal(result, "x");
	mem_free(result);
}

typedef struct s_fail_after_allocator
{
	size_t	attempts;
	size_t	successes;
	size_t	frees;
	size_t	limit;
} t_fail_after_allocator;

static t_fail_after_allocator *active_fail_after;

static void	*fail_after_alloc(
	size_t	size
)
{
	t_fail_after_allocator	*counter = active_fail_after;
	void				*result;

	++counter->attempts;
	if (counter->attempts > counter->limit)
		return (NULL);
	result = malloc(size);
	if (result)
		++counter->successes;
	return (result);
}

static void	fail_after_free(
	void	*ptr
)
{
	t_fail_after_allocator	*counter = active_fail_after;

	++counter->frees;
	free(ptr);
}

static void	test_registry_allocation_failures(void **state)
{
	t_fail_after_allocator counter = {0};
	t_genesis_global_settings settings = {.allocator = {
		.alloc = fail_after_alloc,
		.realloc = NULL, .free = fail_after_free
	}};
	char *result;

	(void)state;
	assert_int_equal(genesis_format_rm(GENESIS_FORMAT_BLTN_COLOR), 0);
	active_fail_after = &counter;
	assert_int_equal(genesis_format_set("BASE", replacement), 0);
	G_SETTINGS = settings;
	counter.limit = 1;
	assert_int_equal(genesis_format_set("PARTIAL", replacement), ENOMEM);
	assert_int_equal(counter.successes, counter.frees);
	G_SETTINGS = (t_genesis_global_settings){
		.allocator = {.alloc = malloc, .free = free, .realloc = realloc}
	};
	result = genesis_format("%{BASE}", (void *)"ignored");
	assert_string_equal(result, "new");
	mem_free(result);
	assert_int_equal(genesis_format_rm("BASE"), 0);
	G_SETTINGS = settings;
	counter.attempts = 0;
	counter.limit = 2;
	assert_int_equal(genesis_format_set("PARTIAL", replacement), ENOMEM);
	assert_int_equal(counter.successes, counter.frees);
	G_SETTINGS = (t_genesis_global_settings){
		.allocator = {.alloc = malloc, .free = free, .realloc = realloc}
	};
	assert_int_equal(genesis_format_rm("PARTIAL"),
		-FORMAT_ERROR_UNKNOWN_TAG);
}

/* ----| Public     |----- */
int	main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(basics),
		cmocka_unit_test(fixed),
		cmocka_unit_test(test_first_tag_after_empty_registry),
		cmocka_unit_test(tags),
		cmocka_unit_test(test_custom_options_expand_in_argument_order),
		cmocka_unit_test(errors),
		cmocka_unit_test(floats_and_n),
		cmocka_unit_test(standard_boundaries),
		cmocka_unit_test(malformed),
		cmocka_unit_test(settings_contract),
		cmocka_unit_test(test_global_allocator_is_global),
		cmocka_unit_test(registry_growth),
		cmocka_unit_test(test_va_forms_and_errors),
		cmocka_unit_test(test_registry_replace_and_invalid),
		cmocka_unit_test(test_allocator_failure),
		cmocka_unit_test(test_callback_failure),
		cmocka_unit_test(test_custom_option_errors_skip_callback),
		cmocka_unit_test(test_standard_conversion_matrix),
		cmocka_unit_test(test_settings_defaults_and_zero),
		cmocka_unit_test(test_registry_allocation_failures)};
	return (cmocka_run_group_tests(tests, NULL, NULL));
}
