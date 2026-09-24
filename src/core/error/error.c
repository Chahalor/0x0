/**
 * @file error.c
 * @brief Public wrappers for core error messages.
 */

/* ----| Headers    |----- */
	/* Internal */
# include "error.h"
# include "_internal_/_error.h"

/* ----| Prototypes |----- */

/* ----| Internals  |----- */

/* ----| Public     |----- */
const char	*core_error_strerror(
	const int	code
)
{
	return (_core_error_strerror(code));
}

const char	*genesis_strerror(
	const t_genesis_core_module	module,
	const int					code
)
{
	return (_genesis_strerror(module, code));
}
