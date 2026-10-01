/**
 * @file _setting.c
 * @brief Validate and publish UUID behavior settings.
 */

/* ----| Headers    |----- */
	/* Standard */
# include <errno.h>
# include <sys/random.h>

	/* Internal */
# include "_uuid.h"

/* ----| State      |----- */
t_genesis_uuid_config uuid_config = {7, getrandom};

/* ----| Internals  |----- */
/** @brief Publish a complete configuration after validation. */
int _uuid_settings_apply(void)
{
	t_genesis_uuid_config config;

	config.default_version = genesis_uuid_setting.default_version;
	config.getrandom = genesis_uuid_setting.getrandom;
	if (config.default_version != 7 || !config.getrandom)
		return (EINVAL);

	uuid_config = config;
	return (0);
}
