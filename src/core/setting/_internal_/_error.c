/**
 * @file _error.c
 * @brief Private setting error messages.
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
/** @brief Messages indexed by setting error enum values. */
static const char *const	_setting_messages[] = {
	[SETTING_ERROR_SUCCESS] = "Success",
	[SETTING_ERROR_NOT_CONFIGURED] = "Setting is not configured",
	[SETTING_ERROR_UNKNOWN] = "Unknown setting error"
};

/** @brief Decode standard errno or index the module message table. */
const char	*_setting_strerror(
	const int	code
)
{
	size_t	index;

	if (code > 0)
		return (strerror(code));

	index = (size_t)(0u - (unsigned int)code);
	if (index >= sizeof(_setting_messages) / sizeof(_setting_messages[0])
		|| !_setting_messages[index])
		index = SETTING_ERROR_UNKNOWN;

	return (_setting_messages[index]);
}
