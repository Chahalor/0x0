/**
 * @file _setting.c
 * @brief Private settings construction and access.
 */

/* ----| Headers    |----- */
	/* Standard */
# include <errno.h>

	/* Internal */
# include "_setting.h"
# include "../setting.h"

	/* External */
# include "../../allocator/allocator.h"

/* ----| State      |----- */
static t_genesis_core_allocator	active_default_allocator;

/* ----| Prototypes |----- */

/* ----| Internals  |----- */
/** @brief Validate and publish the global default allocator. */
int	_settings_global_apply(void)
{
	t_genesis_core_allocator	allocator;

	allocator = genesis_global_settings.allocator;
	if (!allocator.alloc
		&& !allocator.realloc && !allocator.free)
		allocator = core_allocator_system();
	if (core_allocator_validate(&allocator) != 0)
		return (EINVAL);

	active_default_allocator = allocator;
	return (0);
}

/** @brief Return a complete snapshot of the applied global default. */
t_genesis_core_allocator	_settings_global_allocator(void)
{
	if (!active_default_allocator.alloc)
		return (core_allocator_system());

	return (active_default_allocator);
}

/** @brief Construct defaults from the applied global allocator. */
t_genesis_core_settings	_settings_default(void)
{
	return ((t_genesis_core_settings){
		.allocator = _settings_global_allocator()
	});
}

/** @brief Copy the validated descriptor into caller-owned settings. */
void	_settings_set_allocator(
	t_genesis_core_settings			*const	settings,
	const t_genesis_core_allocator	*const	allocator
)
{
	settings->allocator = *allocator;
}

/** @brief Borrow the allocator descriptor from caller-owned settings. */
const t_genesis_core_allocator	*_settings_allocator(
	const t_genesis_core_settings	*const	settings
)
{
	return (&settings->allocator);
}
