/**
 * @file setting.c
 * @brief Public validation and access to core settings.
 */

/* ----| Headers    |----- */
	/* Standard */
# include <errno.h>

	/* Internal */
# include "setting.h"
# include "_internal_/_manager.h"
# include "_internal_/_setting.h"

	/* External */
# include "../allocator/allocator.h"

/* ----| Prototypes |----- */

/* ----| Internals  |----- */

/* ----| Public     |----- */
t_genesis_core_settings	core_settings_default(void)
{
	return (_settings_default());
}

int	core_settings_set_allocator(
	t_genesis_core_settings			*const	settings,
	const t_genesis_core_allocator	*const	allocator
)
{
	if (!settings || core_allocator_validate(allocator) != 0)
		return (EINVAL);

	_settings_set_allocator(settings, allocator);
	return (0);
}

const t_genesis_core_allocator	*core_settings_allocator(
	const t_genesis_core_settings	*const	settings
)
{
	if (!settings)
	{
		errno = EINVAL;
		return (NULL);
	}

	return (_settings_allocator(settings));
}

int	genesis_settings_init(
	t_genesis_settings	*const	settings
)
{
	if (!settings)
		return (EINVAL);

	_manager_init(settings);
	return (0);
}

void	genesis_settings_deinit(
	t_genesis_settings	*const	settings
)
{
	if (!settings)
		return;

	_manager_deinit(settings);
}

int	genesis_settings_set(
	t_genesis_settings			*const	settings,
	const t_genesis_setting_key	*const	key,
	const void					*const	value
)
{
	if (!settings || !key || !key->name || key->size == 0 || !value)
		return (EINVAL);

	return (_manager_set(settings, key, value));
}

int	genesis_settings_get(
	const t_genesis_settings		*const	settings,
	const t_genesis_setting_key	*const	key,
	void						*const	output
)
{
	if (!settings || !key || !key->name || key->size == 0 || !output)
		return (EINVAL);

	return (_manager_get(settings, key, output));
}

int	genesis_settings_reset(
	t_genesis_settings			*const	settings,
	const t_genesis_setting_key	*const	key
)
{
	if (!settings || !key || !key->name || key->size == 0)
		return (EINVAL);

	return (_manager_reset(settings, key));
}
