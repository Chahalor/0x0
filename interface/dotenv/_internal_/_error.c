/**
 * 
*/
/* ----| Headers    |----- */
	/* Standard */
#include "unistd.h"

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
	write(STDERR_FILENO, msg, strlen(msg));
	write(STDERR_FILENO, )
}