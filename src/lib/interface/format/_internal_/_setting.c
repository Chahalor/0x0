/**
 * @file _setting.c
 * @brief Validate, resolve, and publish the format runtime configuration.
 */

/* ----| Headers    |----- */
	/* Standard */
# include <errno.h>
# include <stdint.h>

	/* Internal */
# include "_setting.h"
# include "../setting.h"

/* ----| State      |----- */
static t_format_config	active_config = {
	.max_output	= SIZE_MAX - 1
};

/* ----| Internals  |----- */
/** @brief Build a complete snapshot and publish only after validation. */
int	_format_settings_apply(void)
{
	t_format_config	config;

	config.max_output = genesis_format_settings.max_output;
	if (config.max_output == SIZE_MAX)
		return (EINVAL);

	active_config = config;
	return (0);
}

/** @brief Copy only the last successfully applied configuration. */
int	_format_settings_snapshot(
	t_format_config	*output
)
{
	*output = active_config;
	return (0);
}
