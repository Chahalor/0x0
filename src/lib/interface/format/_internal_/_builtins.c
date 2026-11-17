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
#include "standards/formating.h"

	/* External */
		//...

/* ----| Prototypes |----- */

/* ----| Internals  |----- */

/** @brief Compare a requested name with a sorted builtin color entry. */
static int	_compare_bltn_color(
	const void	*key,
	const void	*entry
)
{
	return (strcmp(key, ((const t_bltn_color *)entry)->name));
}

static int	_parse_rgb(
	const char		*rgb,
	char			*result,
	const size_t	s_result
)
{
	const char	*cursor;
	char		*end;
	long		component;
	size_t		index;
	size_t		length;
	int			colors[3];

	if (!rgb || !result || s_result == 0 || rgb[0] != '[')
		return (-1);

	cursor = rgb + 1;
	for (index = 0; index < 3; ++index)
	{
		while (isspace((unsigned char)*cursor))
			++cursor;
		if (!isdigit((unsigned char)*cursor))
			return (-1);

		component = strtol(cursor, &end, 10);
		if (component > 255)
			return (-1);

		colors[index] = (int)component;
		cursor = end;
		while (isspace((unsigned char)*cursor))
			++cursor;
		if (index < 2)
		{
			if (*cursor != ',')
				return (-1);
			++cursor;
		}
		else if (*cursor != ']' || cursor[1] != '\0')
			return (-1);
	}

	length = genesis_sformat(result, s_result, "\033[38;2;%d;%d;%dm",
		colors[0], colors[1], colors[2]);
	if (length == SIZE_MAX || length >= s_result)
		return (-1);
	return (0);
}

static int	_hex_value(
	const char c
)
{
	if (c >= '0' && c <= '9')
		return (c - '0');
	if (c >= 'a' && c <= 'f')
		return (c - 'a' + 10);
	if (c >= 'A' && c <= 'F')
		return (c - 'A' + 10);
	return (-1);
}

static int	_parse_hexa(
	const char		*hexa,
	char			*result,
	const size_t	s_result
)
{
	int	rgb[3] = {0};
	int	value;
	size_t	len;
	size_t	i;

	if (!hexa || !result || s_result == 0)
		return (-1);

	result[0] = '\0';

	if (hexa[0] != '#' || strlen(hexa) != 7)
		return (-1);

	for (i = 0; i < 6; ++i)
	{
		value = _hex_value(hexa[i + 1]);
		if (value == -1)
			return (-1);
		rgb[i / 2] = rgb[i / 2] * 16 + value;
	}

	len = genesis_sformat(
		result,
		s_result,
		"\033[38;2;%d;%d;%dm",
		rgb[0], rgb[1], rgb[2]
	);

	if (len == SIZE_MAX || len >= s_result)
	{
		result[0] = '\0';
		return (-1);
	}

	return (0);
}

static int	_parse_bltn(
	const char			*bltn,
	const t_bltn_color	*builtins,
	const size_t			count,
	char				*result,
	const size_t		s_result
)
{
	const t_bltn_color	*found;
	size_t					length;

	found = bsearch(bltn, builtins, count, sizeof(*builtins),
		_compare_bltn_color);
	if (!found)
		return (-1);

	length = strlen(found->value);
	if (length >= s_result)
		return (-1);
	memcpy(result, found->value, length + 1);
	return (0);
}

/* ----| Public     |----- */

/**
 * format: `COLOR:NAME or CODE`
 * 
 * @example:
 * %{COLOR:RED}
 * %{COLOR:[255,16,127]}
 * %{COLOR:#5c5749}
 */
char	*genesis_format_builtin_colors(
	const void	*value,
	const char	*param
)
{
	fprintf(stderr, "%s has been caller\n", __func__);	// rm
	static const t_bltn_color	builtins[] = {
		{"AQUA", AQUA},
		{"BEIGE", BEIGE},
		{"BLACK", BLACK},
		{"BLUE", BLUE},
		{"BRIGHT_BLACK", BRIGHT_BLACK},
		{"BRIGHT_BLUE", BRIGHT_BLUE},
		{"BRIGHT_CYAN", BRIGHT_CYAN},
		{"BRIGHT_GREEN", BRIGHT_GREEN},
		{"BRIGHT_MAGENTA", BRIGHT_MAGENTA},
		{"BRIGHT_RED", BRIGHT_RED},
		{"BRIGHT_WHITE", BRIGHT_WHITE},
		{"BRIGHT_YELLOW", BRIGHT_YELLOW},
		{"BROWN", BROWN},
		{"CORAL", CORAL},
		{"CRIMSON", CRIMSON},
		{"CYAN", CYAN},
		{"GOLD", GOLD},
		{"GRAY", GRAY},
		{"GREEN", GREEN},
		{"GREY", GREY},
		{"INDIGO", INDIGO},
		{"KHAKI", KHAKI},
		{"LIME", LIME},
		{"MAGENTA", MAGENTA},
		{"MAROON", MAROON},
		{"NAVY", NAVY},
		{"OLIVE", OLIVE},
		{"ORANGE", ORANGE},
		{"PINK", PINK},
		{"PURPLE", PURPLE},
		{"RED", RED},
		{"SALMON", SALMON},
		{"SILVER", SILVER},
		{"TEAL", TEAL},
		{"TURQUOISE", TURQUOISE},
		{"VIOLET", VIOLET},
		{"WHITE", WHITE},
		{"YELLOW", YELLOW}
	};
	char	*result;
	int		err;

	(void)value;
	if (!param)
	{
		errno = EINVAL;
		return (NULL);
	}

	result = mem_alloc(20);
	if (!result)
		return (NULL);

	if (*param == '[')
		err = _parse_rgb(param, result, 20);
	else if (*param == '#')
		err = _parse_hexa(param, result, 20);
	else
		err = _parse_bltn(param, builtins,
			sizeof(builtins) / sizeof(builtins[0]), result, 20);

	if (err < 0)
	{
		mem_free(result);
		errno = EINVAL;
		return (NULL);
	}

	return (result);
}

void	genesis_format_builtins_init(void)
{
	static char	is_init = 0;

	if (likely(is_init))
		return ;

	fprintf(stderr, "%s has been caller\n", __func__);	// rm
	genesis_format_set("COLOR", genesis_format_builtin_colors);
	is_init = 1;
}
