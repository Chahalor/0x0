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

	/* External */
# include "../../allocator/error.h"
# include "../../buffer/error.h"
# include "../../setting/error.h"

/* ----| Prototypes |----- */

/* ----| Internals  |----- */
/** @brief Messages indexed by core error enum values. */
static const char *const	_core_error_messages[] = {
	[CORE_ERROR_SUCCESS] = "Success",
	[CORE_ERROR_UNKNOWN] = "Unknown core error"
};

/** @brief Module message functions indexed by public module IDs. */
static const char *(*const	_genesis_error_handlers[])(int) = {
	[GENESIS_MODULE_ERROR] = core_error_strerror,
	[GENESIS_MODULE_SETTING] = core_setting_strerror,
	[GENESIS_MODULE_ALLOCATOR] = core_allocator_strerror,
	[GENESIS_MODULE_BUFFER] = core_buffer_strerror
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

/** @brief Index the module handler table and dispatch its error code. */
const char	*_genesis_strerror(
	const t_genesis_core_module	module,
	const int					code
)
{
	size_t	index;

	index = (size_t)module;
	if (index >= sizeof(_genesis_error_handlers)
		/ sizeof(_genesis_error_handlers[0])
		|| !_genesis_error_handlers[index])
		return ("Unknown Genesis module");

	return (_genesis_error_handlers[index](code));
}
