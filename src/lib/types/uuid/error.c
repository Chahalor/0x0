/**
 * @file error.c
 * @brief Public UUID error descriptions and descriptor.
 */

/* ----| Headers    |----- */
	/* Internal */
# include "error.h"
# include "_internal_/_uuid.h"

/* ----| Public     |----- */
const char	*genesis_uuid_strerror(
	int code
)
{
	return (_uuid_strerror(code));
}

const t_genesis_error_module	genesis_uuid_error_module = {
	.name		= "lib.types.uuid",
	.describe	= genesis_uuid_strerror
};
