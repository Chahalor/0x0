/**
 * @file _uuid.c
 * @brief Render UUID values using semicolon-separated format options.
 */

/* ----| Headers    |----- */
	/* Standard */
# include <ctype.h>
# include <errno.h>
# include <stdbool.h>
# include <string.h>

	/* External */
# include "../../../../types/uuid.h"

/* ----| Public     |----- */
/** @brief Apply exact upper and compact options without changing the input. */
char	*genesis_format_builtin_uuid(
	const void	*value,
	const char	*param
)
{
	const char	*start;
	const char	*end;
	char		*result;
	size_t		source;
	size_t		target;
	bool		upper = false;
	bool		compact = false;

	if (!param)
	{
		errno = EINVAL;
		return (NULL);
	}

	start = param;
	if (*start)
	{
		while (true)
		{
			size_t	length;

			end = strchr(start, ';');
			length = end ? (size_t)(end - start) : strlen(start);
			if (length == sizeof("upper") - 1 && memcmp(start, "upper", length) == 0)
				upper = true;
			else if (length == sizeof("compact") - 1 && memcmp(start, "compact", length) == 0)
				compact = true;
			else
			{
				errno = EINVAL;
				return (NULL);
			}

			if (!end)
				break;
			start = end + 1;
		}
	}

	result = genesis_uuid_stringify(value);
	if (!result)
		return (NULL);

	target = 0;
	for (source = 0; result[source]; ++source)
	{
		if (compact && result[source] == '-')
			continue;

		result[target++] = upper
			? (char)toupper((unsigned char)result[source])
			: result[source];
	}

	result[target] = '\0';
	return (result);
}
