#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "interface/log.h"
#include "lib/interface/log/_internal_/_log.h"

static void test_log_init_status(void **state)
{
	t_log_init init = {.display_limit = log_other};

	(void)state;
	assert_int_equal(log_init(NULL), error_invalid_arg);
	assert_int_equal(log_init(&init), 0);
	assert_int_equal(_log_config(NULL)->display_limit, log_other);
}

static void test_perror_summary_and_null_summary(void **state)
{
	int pipefd[2];
	char output[1024] = {0};
	ssize_t size;
	t_log_report report = {.level = log_error, .summary = "open"};

	(void)state;
	assert_int_equal(pipe(pipefd), 0);
	_log_config(NULL)->fd[log_error] = pipefd[1];
	_log_config(NULL)->display_limit = log_other;
	errno = ENOENT;
	assert_int_equal(logs_perror_report(&report), 0);
	assert_string_equal(report.summary, "open");
	close(pipefd[1]);
	size = read(pipefd[0], output, sizeof(output) - 1);
	assert_true(size > 0);
	assert_non_null(strstr(output, "open: "));
	assert_non_null(strstr(output, strerror(ENOENT)));
	close(pipefd[0]);
	_log_config(NULL)->fd[log_error] = -1;
	report.summary = NULL;
	errno = ENOENT;
	assert_int_equal(logs_perror_report(&report), 0);
	assert_null(report.summary);
}

int main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_log_init_status),
		cmocka_unit_test(test_perror_summary_and_null_summary),
	};

	return cmocka_run_group_tests(tests, NULL, NULL);
}
