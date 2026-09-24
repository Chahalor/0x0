/**
 * @file error.c
 * @brief Public allocator error message wrapper.
 */

/* ----| Headers    |----- */
	/* Internal */
# include "error.h"
# include "_internal_/_error.h"

/* ----| Prototypes |----- */

/* ----| Internals  |----- */

/* ----| Public     |----- */
const char	*core_allocator_strerror(
	const int	code
)
{
	return (_allocator_strerror(code));
}

const t_genesis_error_module	genesis_allocator_error_module = {
	.name = "core.allocator",
	.describe = core_allocator_strerror
};
