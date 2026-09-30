/**
 * @file _setting.c
 * @brief Default and validation callbacks for allocator settings.
 */

/* ----| Headers    |----- */
	/* Internal */
# include "_setting.h"

	/* External */
# include "../allocator.h"
# include "../../setting/setting.h"

/* ----| Prototypes |----- */

/* ----| Internals  |----- */
/** @brief Write the applied global default into caller-owned storage. */
int	_allocator_setting_defaults(
	void	*const	output
)
{
	*(t_genesis_core_allocator *)output = genesis_default_allocator();
	return (0);
}

/** @brief Accept descriptors with allocation and release callbacks. */
int	_allocator_setting_validate(
	const void	*const	value
)
{
	return (core_allocator_validate(value));
}
