/**
 * @file _setting.c
 * @brief Private buffer growth setting callbacks.
 */

/* ----| Headers    |----- */
	/* Standard */
# include <errno.h>

	/* Internal */
# include "_setting.h"

/* ----| Prototypes |----- */

/* ----| Internals  |----- */
/** @brief Construct the buffer's default growth policy. */
t_genesis_core_buffer_settings	_buffer_settings_default(void)
{
	return ((t_genesis_core_buffer_settings){
		.initial_capacity = 64,
		.growth = BUFFER_GROWTH_GEOMETRIC
	});
}

/** @brief Write default buffer settings into caller-owned storage. */
int	_buffer_setting_defaults(
	void	*const	output
)
{
	*(t_genesis_core_buffer_settings *)output = _buffer_settings_default();
	return (0);
}

/** @brief Reject invalid starting capacity or growth policy. */
int	_buffer_setting_validate(
	const void	*const	value
)
{
	const t_genesis_core_buffer_settings	*settings;

	settings = value;
	if (settings->initial_capacity == 0
		|| (settings->growth != BUFFER_GROWTH_GEOMETRIC
			&& settings->growth != BUFFER_GROWTH_EXACT))
		return (EINVAL);

	return (0);
}
