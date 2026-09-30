/**
 * @file setting.c
 * @brief Public buffer growth setting key.
 */

/* ----| Headers    |----- */
	/* Internal */
# include "setting.h"

/* ----| Prototypes |----- */

/* ----| Internals  |----- */

/* ----| Public     |----- */
const t_genesis_core_buffer_settings	genesis_buffer_setting = {
		.initial_capacity	= 64,
		.growth				= BUFFER_GROWTH_GEOMETRIC
	};
