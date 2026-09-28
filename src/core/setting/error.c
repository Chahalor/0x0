/**
 * @file error.c
 * @brief Public setting error message wrapper.
 */

/* ----| Headers    |----- */
	/* Internal */
# include "error.h"
# include "_internal_/_error.h"

/* ----| Prototypes |----- */

/* ----| Internals  |----- */

/* ----| Public     |----- */
const char	*core_setting_strerror(
	const int	code
)
{
	return (_setting_strerror(code));
}

const t_genesis_error_module	genesis_setting_error_module = {
	.name = "core.setting",
	.describe = core_setting_strerror
};
