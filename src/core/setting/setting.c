/**
 * @file setting.c
 * @brief Defaults for global Genesis allocator callbacks.
 */

/* ----| Headers    |----- */
	/* Standard */
# include <stdlib.h>

	/* Internal */
# include "setting.h"

/* ----| Prototypes |----- */

/* ----| Internals  |----- */

/* ----| Public     |----- */
t_genesis_global_settings	genesis_global_settings = {
	.allocator	= {
		.alloc		= malloc,
		.free		= free,
		.realloc	= realloc
	}
};
