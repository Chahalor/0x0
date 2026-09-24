/**
 * @file setting.c
 * @brief Public buffer growth setting key.
 */

/* ----| Headers    |----- */
	/* Internal */
# include "setting.h"
# include "_internal_/_setting.h"

/* ----| Prototypes |----- */

/* ----| Internals  |----- */

/* ----| Public     |----- */
const t_genesis_setting_key	genesis_buffer_setting = {
	.name = "core.buffer",
	.size = sizeof(t_genesis_core_buffer_settings),
	.defaults = _buffer_setting_defaults,
	.validate = _buffer_setting_validate
};
