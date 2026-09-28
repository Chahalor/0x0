/**
 * @file _error.c
 * @brief Private allocator error messages.
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
/** @brief Messages indexed by allocator error enum values. */
static const char *const	_allocator_messages[] = {
	[ALLOCATOR_ERROR_SUCCESS] = "Success",
	[ALLOCATOR_ERROR_UNKNOWN] = "Unknown allocator error"
};

/** @brief Decode standard errno or index the module message table. */
const char	*_allocator_strerror(
	const int	code
)
{
	size_t	index;

	if (code > 0)
		return (strerror(code));

	index = (size_t)(0u - (unsigned int)code);
	if (index >= sizeof(_allocator_messages) / sizeof(_allocator_messages[0])
		|| !_allocator_messages[index])
		index = ALLOCATOR_ERROR_UNKNOWN;

	return (_allocator_messages[index]);
}
