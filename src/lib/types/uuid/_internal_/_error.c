/**
 * @file _error.c
 * @brief UUID error message lookup.
 */

/* ----| Headers    |----- */
	/* Standard */
# include <stddef.h>
# include <string.h>

	/* Internal */
# include "_uuid.h"

/* ----| State      |----- */
static const char *const uuid_messages[] = {
	[UUID_ERROR_SUCCESS]				= "Success",
	[UUID_ERROR_INVALID_FORMAT]			= "Invalid UUID format",
	[UUID_ERROR_UNSUPPORTED_VERSION]	= "Unsupported UUID version",
	[UUID_ERROR_UNKNOWN]				= "Unknown UUID error"
};

/* ----| Internals  |----- */
/** @brief Map positive errno and negative UUID codes to borrowed messages. */
const char *_uuid_strerror(int code)
{
	size_t index;

	if (code > 0)
		return (strerror(code));

	index = (size_t)(0u - (unsigned int)code);
	if (index >= sizeof(uuid_messages) / sizeof(uuid_messages[0]) || !uuid_messages[index])
		index = UUID_ERROR_UNKNOWN;

	return (uuid_messages[index]);
}
