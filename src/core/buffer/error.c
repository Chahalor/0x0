/**
 * @file error.c
 * @brief Public buffer error message wrapper.
 */

/* ----| Headers    |----- */
	/* Internal */
# include "error.h"
# include "_internal_/_error.h"

/* ----| Prototypes |----- */

/* ----| Internals  |----- */

/* ----| Public     |----- */
const char	*core_buffer_strerror(
	const int	code
)
{
	return (_buffer_strerror(code));
}

const t_genesis_error_module	genesis_buffer_error_module = {
	.name		= "core.buffer",
	.describe	= core_buffer_strerror
};
