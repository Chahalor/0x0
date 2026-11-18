// Header

/* ----| Headers    |----- */
	/* Standard */
		//...

	/* Internal */
#include "../_format.h"

	/* External */
#include "../../../../files/toml.h"

/* ----| Prototypes |----- */
	//...

/* ----| Internals  |----- */
	//...

/* ----| Public     |----- */

/**
 * params:
 * - int -> le pretty du json
*/
char	*genesis_format_builtin_toml(
	const void	*value,
	const char	*param
)
{
	const TOML	*toml = (const TOML *)value;
	char		*result = NULL;
	int			pretty = 0;

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

	result = toml_stringify(toml, pretty);

	return (result);
}