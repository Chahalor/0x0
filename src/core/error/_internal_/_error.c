/**
 * @file _error.c
 * @brief Core error descriptions and module dispatch.
 */

/* ----| Headers    |----- */
	/* Standard */
# include <stddef.h>
# include <string.h>

	/* Internal */
# include "_error.h"
# include "../error.h"

/* ----| Prototypes |----- */

/* ----| Internals  |----- */
/** @brief Messages indexed by core error enum values. */
static const char *const	_core_error_messages[] = {
	[CORE_ERROR_SUCCESS] = "Success",
	[CORE_ERROR_UNKNOWN] = "Unknown core error"
};

/** @brief Decode standard errno or index the core message table. */
const char	*_core_error_strerror(
	const int	code
)
{
	size_t	index;

	if (code > 0)
		return (strerror(code));

	index = (size_t)(0u - (unsigned int)code);
	if (index >= sizeof(_core_error_messages) / sizeof(_core_error_messages[0])
		|| !_core_error_messages[index])
		index = CORE_ERROR_UNKNOWN;

	return (_core_error_messages[index]);
}

/** @brief Dispatch through any module's stable message descriptor. */
const char	*_genesis_strerror(
	const t_genesis_error_module	*const	module,
	const int							code
)
{
	if (!module || !module->name || !module->describe)
		return ("Unknown Genesis module");

	return (module->describe(code));
}
