/**
 * 
*/
/* ----| Headers    |----- */
	/* Standard */
#include <unistd.h>
#include <string.h>

	/* Internal */
#include "_dotenv.h"

	/* External */
		//...

/* ----| Prototypes |----- */
	//...

/* ----| Internals  |----- */
	//...

/* ----| Public     |----- */

const char	*_dotenv_strerror(
	t_interface_dotenv_error code
)
{
	const char	*strs[dotenv_error_unknow + 1] = {
		[dotenv_error_success]		= "Success",
		[dotenv_error_system]		= "System error",
		[dotenv_error_syntax]		= "Invalid Syntax",
		[dotenv_error_invalid_key]	= "Invalid Key",
		[dotenv_error_missing_key]	= "Missing key",
		[dotenv_error_empty_value]	= "Empty value",
		[dotenv_error_unknow]		= "Unknow",
	};

	if (code < dotenv_error_success || code > dotenv_error_unknow)
		code = dotenv_error_unknow;

	return (strs[code]);
}

void	_dotenv_perror(
	const char	*const restrict		msg,
	const t_interface_dotenv_error	code
)
{
	const char	*description = _dotenv_strerror(code);

	if (msg && *msg)
	{
		(void)write(STDERR_FILENO, msg, strlen(msg));
		(void)write(STDERR_FILENO, ": ", 2);
	}
	(void)write(STDERR_FILENO, description, strlen(description));
	(void)write(STDERR_FILENO, "\n", 1);
}
