/**
 * @file _error.c
 * @brief Message table and lookup for format errors.
 */

/* ----| Headers    |----- */
	/* Standard */
# include <stddef.h>
# include <string.h>

	/* Internal */
# include "_format.h"
# include "../types.h"

/* ----| State      |----- */
static const char *const	format_messages[] = {
	[FORMAT_ERROR_SUCCESS]			= "Success",
	[FORMAT_ERROR_INVALID_FORMAT]	= "Invalid format",
	[FORMAT_ERROR_UNKNOWN_TAG]		= "Unknown custom tag",
	[FORMAT_ERROR_UNKNOWN]			= "Unknown format error"
};

/* ----| Internals  |----- */
/** @brief Index module errors and forward positive errno values to libc. */
const char	*_format_strerror(
	int	code
)
{
	size_t	index;

	if (code > 0)
		return (strerror(code));

	index = (size_t)(0u - (unsigned int)code);
	if (index >= sizeof(format_messages) / sizeof(format_messages[0])
		|| !format_messages[index])
		index = FORMAT_ERROR_UNKNOWN;

	return (format_messages[index]);
}
