// Header

/* ----| Headers    |----- */
	/* Standard */
		//...

	/* Internal */
#include "_uuid.h"

	/* External */
#include "interface/format.h"

/* ----| Prototypes |----- */
	//...

/* ----| Internals  |----- */

static bool	uuid_separator__(
	const size_t	byte
)
{
	return (byte == 4 || byte == 6 || byte == 8 || byte == 10);
}

static inline bool	uuid_string_separator__(
	const size_t	index
)
{
	return (
		index == 8
		|| index == 13
		|| index == 18
		|| index == 23
	);
}

static inline bool	is_valid_hex__(
	const char	c
)
{
	return (
		(c >= '0' && c <= '9')
		|| (c >= 'A' && c <= 'F')
		|| (c >= 'a' && c <= 'f')
	);
}

static inline uint8_t	hex_value__(
	const char	c
)
{
	if (c >= '0' && c <= '9')
		return ((uint8_t)(c - '0'));

	else if (c >= 'A' && c <= 'F')
		return ((uint8_t)(c - 'A' + 10));

	return ((uint8_t)(c - 'a' + 10));
}

static inline uint8_t	hex_byte__(
	const char	*const	s
)
{
	return ((uint8_t)(
		(hex_value__(s[0]) << 4)
		| hex_value__(s[1])
	));
}

static inline bool	is_valid_hexa__(
	const char	c
)
{
	// TODO: check for bitmask
	return (
		(c >= '0' && c <= '9') ||
		(c >= 'A' && c <= 'Z') ||
		(c >= 'a' && c <= 'z')
	);
}

/* ----| Public     |----- */

int	uuid_parse_(
	UUID		*const	dest,
	const char	*const	str
)
{
	UUID	tmp;
	size_t	src;
	size_t	dst;

	if (unlikely(!dest || !str))
		return (EINVAL);

	if (unlikely(strlen(str) != UUID_STRING_SIZE - 1))
		return (TYPES_ERROR_UUID_INVALID_FORMAT);

	src = 0;
	dst = 0;
	while (dst < sizeof(tmp.bytes))
	{
		if (unlikely(uuid_string_separator__(src)))
		{
			if (unlikely(str[src] != '-'))
				return (TYPES_ERROR_UUID_INVALID_FORMAT);
			src++;
		}

		if (unlikely(!is_valid_hex__(str[src]) || !is_valid_hex__(str[src + 1])))
			return (TYPES_ERROR_UUID_INVALID_FORMAT);

		tmp.bytes[dst] = hex_byte__(&str[src]);

		src += 2;
		dst++;
	}

	*dest = tmp;
	return (error_none);
}

// 8-4-4-4-12
void	uuid_fill_string_(
	const UUID	*const	uuid,
	char		*const	r
)
{
	static const char	hex[17] = "0123456789abcdef";
	size_t				src = 0;
	size_t				dst = 0;

	while (src < 16)
	{
		if (unlikely(uuid_separator__(src)))
			r[dst++] = '-';

		r[dst++] = hex[uuid->bytes[src] >> 4];
		r[dst++] = hex[uuid->bytes[src] & 0x0f];
		src++;
	}
	r[dst] = '\0';
}

char	*uuid_stringify_(
	const UUID	*const	uuid
)
{
	char	*r = NULL;

	r = G_SETTINGS.allocator.alloc(sizeof(char) * UUID_STRING_SIZE);
	if (unlikely(!r))
		return (NULL);

	uuid_fill_string_(uuid, r);

	return (r);
}
