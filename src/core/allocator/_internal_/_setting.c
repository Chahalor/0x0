/**
 * @file _setting.c
 * @brief Default and validation callbacks for allocator settings.
 */

/* ----| Headers    |----- */
	/* Internal */
# include "_setting.h"

	/* External */
# include "../allocator.h"

/* ----| Prototypes |----- */

/* ----| Internals  |----- */
/** @brief Write the system allocator into caller-owned output storage. */
int	_allocator_setting_defaults(
	void	*const	output
)
{
	*(t_genesis_core_allocator *)output = core_allocator_system();
	return (0);
}

/** @brief Accept descriptors with allocation and release callbacks. */
int	_allocator_setting_validate(
	const void	*const	value
)
{
	return (core_allocator_validate(value));
}
