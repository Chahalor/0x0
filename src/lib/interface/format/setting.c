/**
 * @file setting.c
 * @brief Public format settings workspace and apply entry point.
 */

/* ----| Headers    |----- */
	/* Standard */
# include <stdint.h>

	/* Internal */
# include "setting.h"
# include "_internal_/_setting.h"

/* ----| Public     |----- */
t_genesis_format_settings	genesis_format_settings = {
	.max_output	= SIZE_MAX - 1,
	.apply		= genesis_format_settings_apply
};

int	genesis_format_settings_apply(void)
{
	return (_format_settings_apply());
}
