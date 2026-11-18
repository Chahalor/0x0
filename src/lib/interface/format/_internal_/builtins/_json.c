// Header

/* ----| Headers    |----- */
	/* Standard */
#include <stdlib.h>

	/* Internal */
#include "../_format.h"

	/* External */
#include "../../../../files/json.h"

/* ----| Prototypes |----- */
	//...

/* ----| Internals  |----- */
	//...

/* ----| Public     |----- */

/**
 * params:
 * - int -> le pretty du json
*/
char	*genesis_format_builtin_json(
	const void	*value,
	const char	*param
)
{
	const JSON	*json = (JSON *)value;
	int			pretty = 0;
	char		*result = NULL;

	if (param && *param)
	{
		char	*end;
		long	r;

		r = strtol(param, &end, 10);
		if (end)
			pretty = 0;
		else
			pretty = (int)r;
	}

	result = json_stringify((JSON *)json, pretty);

	return (result);
}
