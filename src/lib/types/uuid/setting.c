/**
 * @file setting.c
 * @brief Public UUID settings workspace and apply wrapper.
 */

/* ----| Headers    |----- */
	/* Standard */
# include <sys/random.h>

	/* Internal */
# include "setting.h"
# include "_internal_/_uuid.h"

/* ----| Public     |----- */
t_genesis_uuid_setting	genesis_uuid_setting = {
	.default_version = 7,
	.getrandom = getrandom,
	.apply = genesis_uuid_settings_apply
};

int	genesis_uuid_settings_apply(void)
{
	return (_uuid_settings_apply());
}
