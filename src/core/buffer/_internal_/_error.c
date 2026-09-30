/**
 * @file _error.c
 * @brief Private buffer error messages.
 */

/* ----| Headers    |----- */
	/* Standard */
# include <stddef.h>
# include <string.h>

	/* Internal */
# include "_error.h"
# include "../types.h"

/* ----| Prototypes |----- */

/* ----| Internals  |----- */
/** @brief Messages indexed by buffer error enum values. */
static const char *const	_buffer_messages[] = {
	[BUFFER_ERROR_SUCCESS]				= "Success",
	[BUFFER_ERROR_SOURCE_OUT_OF_RANGE]	= "Source range is outside the buffer contents",
	[BUFFER_ERROR_UNKNOWN]				= "Unknown buffer error"
};

/** @brief Decode standard errno or index the module message table. */
const char	*_buffer_strerror(
	const int	code
)
{
	size_t	index;

	if (code > 0)
		return (strerror(code));

	index = (size_t)(0u - (unsigned int)code);
	if (index >= sizeof(_buffer_messages) / sizeof(_buffer_messages[0]) || !_buffer_messages[index])
		index = BUFFER_ERROR_UNKNOWN;

	return (_buffer_messages[index]);
}
