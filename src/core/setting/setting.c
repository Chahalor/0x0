/**
 * @file setting.c
 * @brief Public validation and access to core settings.
 */

/* ----| Headers    |----- */
	/* Standard */
# include <errno.h>

	/* Internal */
# include "setting.h"
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
