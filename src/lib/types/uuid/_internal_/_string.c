/**
 * @file _string.c
 * @brief Parse and render canonical UUID text.
 */

/* ----| Headers    |----- */
	/* Standard */
# include <errno.h>
# include <string.h>

	/* Internal */
# include "_uuid.h"

	/* Modules */
# include "memory.h"

/* ----| Local      |----- */
/** @brief Locate the four canonical separator offsets. */
static int is_separator(
	size_t index
)
{
	return (index == 8 || index == 13 || index == 18 || index == 23);
}

/** @brief Convert one ASCII hex digit to its nibble value. */
static int hex_value(
	char c
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

/* ----| Internals  |----- */
/** @brief Parse into a temporary value so invalid text preserves dest. */
int uuid_parse_(
	UUID *dest,
	const char *str
)
{
	UUID	parsed;
	size_t			src;
	size_t			dst;
	int				high;
	int				low;

	if (!dest || !str)
		return (EINVAL);
	if (strlen(str) != UUID_STRING_SIZE - 1)
		return (-UUID_ERROR_INVALID_FORMAT);
	src = 0;
	for (dst = 0; dst < sizeof(parsed.bytes); ++dst)
	{
		if (is_separator(src))
		{
			if (str[src++] != '-')
				return (-UUID_ERROR_INVALID_FORMAT);
		}
		high = hex_value(str[src++]);
		low = hex_value(str[src++]);
		if (high < 0 || low < 0)
			return (-UUID_ERROR_INVALID_FORMAT);
		parsed.bytes[dst] = (uint8_t)((high << 4) | low);
	}
	*dest = parsed;
	return (0);
}

/** @brief Write exactly 36 canonical characters and one terminator. */
void uuid_fill_string_(
	const UUID *uuid,
	char *result
)
{
	static const char	hex[] = "0123456789abcdef";
	size_t			src;
	size_t			dst;

	dst = 0;
	for (src = 0; src < sizeof(uuid->bytes); ++src)
	{
		if (src == 4 || src == 6 || src == 8 || src == 10)
			result[dst++] = '-';

		result[dst++] = hex[uuid->bytes[src] >> 4];
		result[dst++] = hex[uuid->bytes[src] & 0x0f];
	}
	result[dst] = '\0';
}

/** @brief Allocate a tracked canonical string. */
char *uuid_stringify_(
	const UUID *uuid
)
{
	char	*result;

	result = mem_alloc(UUID_STRING_SIZE);
	if (!result)
		return (NULL);
	uuid_fill_string_(uuid, result);
	return (result);
}
