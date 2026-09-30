/**
 * @file error.c
 * @brief Public format error descriptions and descriptor.
 */

/* ----| Headers    |----- */
	/* Internal */
# include "error.h"
# include "_internal_/_format.h"

/* ----| Public     |----- */
const char	*genesis_format_strerror(
	int	code
)
{
	return (_format_strerror(code));
}

const t_genesis_error_module	genesis_format_error_module = {
	.name		= "lib.interface.format",
	.describe	= genesis_format_strerror
};
