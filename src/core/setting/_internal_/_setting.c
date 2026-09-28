/**
 * @file _setting.c
 * @brief Private settings construction and access.
 */

/* ----| Headers    |----- */
	/* Internal */
# include "_setting.h"

	/* External */
# include "../../allocator/allocator.h"

/* ----| Prototypes |----- */

/* ----| Internals  |----- */
/** @brief Construct defaults from the system allocator. */
t_genesis_core_settings	_settings_default(void)
{
	return ((t_genesis_core_settings){
		.allocator = core_allocator_system()
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
