// Header

/* ----| Headers    |----- */
	/* Standard */
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

	/* Internal */
#include "_format.h"
#include "../../format.h"

	/* External */
		//...

/* ----| Prototypes |----- */

/* ----| Internals  |----- */

/* ----| Public     |----- */

void	genesis_format_builtins_init(void)
{
	static char	is_init = 0;

	if (likely(is_init))
		return ;

	_format_set_safe(GENESIS_FORMAT_BLTN_COLOR,	genesis_format_builtin_colors);
	genesis_format_set(GENESIS_FORMAT_UUID,		genesis_format_builtin_uuid);
	genesis_format_set(GENESIS_FORMAT_JSON,		genesis_format_builtin_json);
	genesis_format_set(GENESIS_FORMAT_TOML,		genesis_format_builtin_toml);
	is_init = 1;
}
