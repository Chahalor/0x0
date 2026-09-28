/**
 * @file setting.c
 * @brief Public allocator setting key.
 */

/* ----| Headers    |----- */
	/* Internal */
# include "setting.h"
# include "_internal_/_setting.h"

/* ----| Prototypes |----- */

/* ----| Internals  |----- */

/* ----| Public     |----- */
const t_genesis_setting_key	genesis_allocator_setting = {
	.name = "core.allocator",
	.size = sizeof(t_genesis_core_allocator),
	.defaults = _allocator_setting_defaults,
	.validate = _allocator_setting_validate
};
