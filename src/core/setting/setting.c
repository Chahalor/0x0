/**
 * @file setting.c
 * @brief Public validation and access to core settings.
 */

/* ----| Headers    |----- */
	/* Standard */
# include <errno.h>
# include <stdlib.h>

	/* Internal */
# include "setting.h"
# include "_internal_/_setting.h"

	/* External */
	//...

/* ----| Prototypes |----- */

/* ----| Internals  |----- */

/* ----| Public     |----- */
t_genesis_global_settings	genesis_global_settings = {
	.allocator	= {
		.alloc		= malloc,
		.free		= free,
		.realloc	= realloc
	},
	// .apply		= genesis_global_settings_apply
};
