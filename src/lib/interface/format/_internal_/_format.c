/**
 * @file _format.c
 * @brief Central formatter state and conversion engine.
 */

/* ----| Headers    |----- */
	/* Standard */
# include <errno.h>
# include <inttypes.h>
# include <limits.h>
# include <stdint.h>
# include <stdio.h>
# include <string.h>
# include <sys/types.h>
# include <wchar.h>
# include <stdbool.h>

	/* Internal */
# include "_format.h"

	/* External */
# include "../../../memory.h"

/* ----| Prototypes |----- */
static int	emit(
	t_genesis_format_	*cfg,
	const char			*data,
	size_t				length
);
static int	parse_spec(
	t_genesis_format_	*cfg,
	t_spec				*spec
);
static int	render_standard(
	t_genesis_format_	*cfg,
	const t_spec		*spec
);
static int	render_custom(
	t_genesis_format_	*cfg
);

/* ----| Internals  |----- */
/** @brief Grow or truncate output while tracking full length. */
static int	emit(
	t_genesis_format_	*cfg,
	const char			*data,
	size_t				length
)
{
	size_t available;
	size_t take;
	size_t needed;
	char  *grown;

	if (length > cfg->max_output - cfg->index)
		return (EOVERFLOW);
	if (length > SIZE_MAX - cfg->index - 1)
		return (EOVERFLOW);
	needed = cfg->index + length + 1;
	if (!cfg->fixed_capacity && needed > cfg->capacity)
	{
		size_t capacity = cfg->capacity ? cfg->capacity : 64;
		while (capacity < needed)
		{
			if (capacity > SIZE_MAX / 2)
			{
				capacity = needed;
				break;
			}
			capacity *= 2;
		}
		grown = mem_realloc(cfg->buffer, capacity);
		if (!grown)
			return (errno);
		cfg->buffer = grown;
		cfg->capacity = capacity;
	}
	if (cfg->fixed_capacity)
	{
		available = cfg->capacity && cfg->index < cfg->capacity - 1
						? cfg->capacity - cfg->index - 1
						: 0;
		take	  = length < available ? length : available;
		if (take)
			memcpy(cfg->buffer + cfg->index, data, take);
		if (cfg->capacity)
		{
			size_t end = cfg->index + take;
			if (end >= cfg->capacity)
				end = cfg->capacity - 1;
			cfg->buffer[end] = '\0';
		}
	}
	else
	{
		if (length)
			memcpy(cfg->buffer + cfg->index, data, length);
		cfg->buffer[cfg->index + length] = '\0';
	}
	cfg->index += length;
	return (0);
}

/** @brief Append one byte to the bounded libc fallback specifier. */
static int	append_char(
	t_spec	*spec,
	char	c
)
{
	if (spec->size >= sizeof(spec->text) - 1)
		return (EOVERFLOW);
	spec->text[spec->size++] = c;
	spec->text[spec->size]	 = '\0';
	return (0);
}

/** @brief Append an unsigned width or precision to a specifier. */
static int	append_decimal(
	t_spec			*spec,
	unsigned int	value
)
{
	char   digits[16];
	size_t length = 0;
	while (value)
	{
		digits[length++] = (char)('0' + value % 10);
		value /= 10;
	}
	if (length == 0)
		digits[length++] = '0';
	while (length)
	{
		int status = append_char(spec, digits[--length]);
		if (status)
			return (status);
	}
	return (0);
}

/** @brief Consume one standard conversion and resolve star arguments. */
static int	parse_spec(
	t_genesis_format_	*cfg,
	t_spec				*spec
)
{
	const char *fm = cfg->fm;
	size_t		at = cfg->cursor;
	int			width;
	int			status;
	char		c;

	memset(spec, 0, sizeof(*spec));
	spec->precision = -1;
	status			= append_char(spec, '%');
	if (status)
		return (status);
	while ((c = fm[at]) && strchr("-+ #0", c))
	{
		if (c == '-')
			spec->left = true;
		if (c == '+')
			spec->plus = true;
		if (c == ' ')
			spec->space = true;
		if (c == '0')
			spec->zero = true;
		if (c == '#')
			spec->alternate = true;
		if ((status = append_char(spec, c)))
			return (status);
		++at;
	}
	if (fm[at] == '*')
	{
		width = va_arg(cfg->list, int);
		++at;
		if (width < 0)
		{
			if ((status = append_char(spec, '-')))
				return (status);
			if (width == INT_MIN)
				return (EOVERFLOW);
			width	   = -width;
			spec->left = true;
		}
		spec->width = (unsigned int)width;
		if ((status = append_decimal(spec, (unsigned int)width)))
			return (status);
	}
	else
		while (fm[at] >= '0' && fm[at] <= '9')
		{
			if (spec->width > (unsigned int)INT_MAX / 10)
				return (EOVERFLOW);
			spec->width = spec->width * 10 + (unsigned int)(fm[at] - '0');
			if (spec->width > INT_MAX)
				return (EOVERFLOW);
			if ((status = append_char(spec, fm[at++])))
				return (status);
		}
	if (fm[at] == '.')
	{
		++at;
		if (fm[at] == '*')
		{
			width = va_arg(cfg->list, int);
			++at;
			if (width >= 0)
			{
				spec->precision = width;
				if ((status = append_char(spec, '.')))
					return (status);
				if ((status = append_decimal(spec, (unsigned int)width)))
					return (status);
			}
		}
		else
		{
			spec->precision = 0;
			if ((status = append_char(spec, '.')))
				return (status);
			while (fm[at] >= '0' && fm[at] <= '9')
			{
				if (spec->precision > INT_MAX / 10)
					return (EOVERFLOW);
				spec->precision = spec->precision * 10 + fm[at] - '0';
				if (spec->precision < 0)
					return (EOVERFLOW);
				if ((status = append_char(spec, fm[at++])))
					return (status);
			}
		}
	}
	if (strchr("hljztL", fm[at]) && fm[at])
	{
		spec->length[0] = fm[at];
		if ((status = append_char(spec, fm[at++])))
			return (status);
		if ((spec->length[0] == 'h' || spec->length[0] == 'l') &&
			fm[at] == spec->length[0])
		{
			spec->length[1] = fm[at];
			if ((status = append_char(spec, fm[at++])))
				return (status);
		}
	}
	c = fm[at];
	if (!c || !strchr("diouxXfFeEgGaAcspn%", c))
		return (EINVAL);
	if ((status = append_char(spec, c)))
		return (status);
	if (strchr("diouxX", c) && spec->length[0] == 'L')
		return (EINVAL);
	if (strchr("fFeEgGaA", c) && spec->length[0] && spec->length[0] != 'l' &&
		spec->length[0] != 'L')
		return (EINVAL);
	if (strchr("cs", c) && spec->length[0] &&
		(spec->length[0] != 'l' || spec->length[1]))
		return (EINVAL);
	if (c == 'c' && spec->precision >= 0)
		return (EINVAL);
	if (strchr("cs", c) &&
		(spec->plus || spec->space || spec->zero || spec->alternate))
		return (EINVAL);
	if (c == 'p' && spec->length[0])
		return (EINVAL);
	if (c == 'n' && (spec->precision >= 0 || spec->width || spec->left ||
					 spec->plus || spec->space || spec->zero ||
					 spec->alternate || spec->length[0] == 'L'))
		return (EINVAL);
	if (c == '%' &&
		(spec->length[0] || spec->precision >= 0 || spec->width || spec->left ||
		 spec->plus || spec->space || spec->zero || spec->alternate))
		return (EINVAL);
	spec->conversion = c;
	cfg->cursor		 = at + 1;
	return (0);
}

/** @brief Emit padding, skipping materialization after fixed truncation. */
static int	emit_repeat(
	t_genesis_format_	*cfg,
	char				c,
	size_t				count
)
{
	char   block[64];
	size_t part;
	int	   status;

	memset(block, c, sizeof(block));
	while (count)
	{
		if (cfg->fixed_capacity &&
			(cfg->capacity == 0 || cfg->index >= cfg->capacity - 1))
		{
			if (count > cfg->max_output - cfg->index ||
				count > SIZE_MAX - cfg->index - 1)
				return (EOVERFLOW);
			cfg->index += count;
			return (0);
		}
		part   = count < sizeof(block) ? count : sizeof(block);
		status = emit(cfg, block, part);
		if (status)
			return (status);
		count -= part;
	}
	return (0);
}

/** @brief Apply left or right padding around narrow text. */
static int	emit_padded(
	t_genesis_format_	*cfg,
	const t_spec		*spec,
	const char			*data,
	size_t				length
)
{
	size_t padding = spec->width > length ? spec->width - length : 0;
	int	   status;

	if (!spec->left)
	{
		status = emit_repeat(cfg, ' ', padding);
		if (status)
			return (status);
	}
	status = emit(cfg, data, length);
	if (status)
		return (status);
	if (spec->left)
		return (emit_repeat(cfg, ' ', padding));
	return (0);
}

/** @brief Render sign, radix, precision and field width without libc. */
static int	render_integer(
	t_genesis_format_	*cfg,
	const t_spec		*spec,
	uintmax_t			magnitude,
	bool				negative
)
{
	char		digits[sizeof(uintmax_t) * CHAR_BIT + 1];
	char		prefix[3];
	const char *alphabet =
		spec->conversion == 'X' ? "0123456789ABCDEF" : "0123456789abcdef";
	unsigned int base =
		spec->conversion == 'o'
			? 8
			: (spec->conversion == 'x' || spec->conversion == 'X' ? 16 : 10);
	bool   nonzero		 = magnitude != 0;
	size_t length		 = 0;
	size_t prefix_length = 0;
	size_t zeros;
	size_t total;
	size_t spaces;
	int	   status;

	if (magnitude || spec->precision != 0)
	{
		do
		{
			digits[length++] = alphabet[magnitude % base];
			magnitude /= base;
		} while (magnitude);
	}
	if (negative)
		prefix[prefix_length++] = '-';
	else if (spec->conversion == 'd' || spec->conversion == 'i')
	{
		if (spec->plus)
			prefix[prefix_length++] = '+';
		else if (spec->space)
			prefix[prefix_length++] = ' ';
	}
	if (spec->alternate && base == 16 && nonzero)
	{
		prefix[prefix_length++] = '0';
		prefix[prefix_length++] = spec->conversion == 'X' ? 'X' : 'x';
	}
	zeros = spec->precision > 0 && (size_t)spec->precision > length
				? (size_t)spec->precision - length
				: 0;
	if (spec->alternate && base == 8 &&
		(length == 0 || digits[length - 1] != '0') && zeros == 0)
		zeros = 1;
	total  = prefix_length + zeros + length;
	spaces = spec->width > total ? spec->width - total : 0;
	if (spec->zero && !spec->left && spec->precision < 0)
	{
		zeros += spaces;
		spaces = 0;
	}
	if (!spec->left && (status = emit_repeat(cfg, ' ', spaces)))
		return (status);
	if (prefix_length && (status = emit(cfg, prefix, prefix_length)))
		return (status);
	if ((status = emit_repeat(cfg, '0', zeros)))
		return (status);
	while (length)
	{
		if ((status = emit(cfg, &digits[--length], 1)))
			return (status);
	}
	if (spec->left)
		return (emit_repeat(cfg, ' ', spaces));
	return (0);
}

/** @brief Render one narrow character or bounded string. */
static int	render_text(
	t_genesis_format_	*cfg,
	const t_spec		*spec
)
{
	const char *text;
	char		character;
	size_t		length;

	if (spec->conversion == 'c')
	{
		character = (char)va_arg(cfg->list, int);
		return (emit_padded(cfg, spec, &character, 1));
	}
	text = va_arg(cfg->list, char *);
	if (!text)
		return (EINVAL);
	length = 0;
	while (text[length] &&
		   (spec->precision < 0 || length < (size_t)spec->precision))
		++length;
	return (emit_padded(cfg, spec, text, length));
}

/* Render a single typed argument. libc is used only for numeric conversion;
 * the module itself controls parsing, tag lookup, output and ownership. */
#define RENDER_TYPED(type)                                                     \
	do                                                                         \
	{                                                                          \
		type   value = va_arg(cfg->list, type);                                \
		size_t space;                                                          \
		int	   written = snprintf(NULL, 0, spec->text, (value));               \
		if (written < 0)                                                       \
			return (EINVAL);                                                   \
		if ((size_t)written > cfg->max_output - cfg->index)                    \
			return (EOVERFLOW);                                                \
		space = (size_t)written + 1;                                           \
		if (space == 0)                                                        \
			return (EOVERFLOW);                                                \
		piece = mem_alloc(space);                                 \
		if (!piece)                                                            \
			return (errno);                                                    \
		if (snprintf(piece, space, spec->text, (value)) != written)            \
		{                                                                      \
			mem_free(piece);                               \
			return (EINVAL);                                                   \
		}                                                                      \
		status = emit(cfg, piece, (size_t)written);                            \
		mem_free(piece);                                   \
		return (status);                                                       \
	} while (0)

/** @brief Dispatch a parsed standard conversion by its argument type. */
static int	render_standard(
	t_genesis_format_	*cfg,
	const t_spec		*spec
)
{
	char					*piece;
	int						 status;
	char					 c = spec->conversion;
	char					 l = spec->length[0];

	if (c == '%')
		return (emit(cfg, "%", 1));
	if (c == 'n')
	{
		if (cfg->index > INT_MAX)
			return (EOVERFLOW);
#define STORE_COUNT(type)                                                      \
	do                                                                         \
	{                                                                          \
		type *target = va_arg(cfg->list, type *);                              \
		if (!target)                                                           \
			return (EINVAL);                                                   \
		*target = (type)cfg->index;                                            \
		return (0);                                                            \
	} while (0)
		if (l == 'h' && spec->length[1] == 'h')
			STORE_COUNT(signed char);
		if (l == 'h')
			STORE_COUNT(short);
		if (l == 'l' && spec->length[1] == 'l')
			STORE_COUNT(long long);
		if (l == 'l')
			STORE_COUNT(long);
		if (l == 'j')
			STORE_COUNT(intmax_t);
		if (l == 'z')
			STORE_COUNT(ssize_t);
		if (l == 't')
			STORE_COUNT(ptrdiff_t);
		STORE_COUNT(int);
#undef STORE_COUNT
	}
	if (strchr("diouxX", c))
	{
		bool	  signed_conversion = c == 'd' || c == 'i';
		intmax_t  signed_value		= 0;
		uintmax_t value				= 0;
		bool	  negative			= false;

		if (signed_conversion)
		{
			if (l == 'l' && spec->length[1] == 'l')
				signed_value = va_arg(cfg->list, long long);
			else if (l == 'l')
				signed_value = va_arg(cfg->list, long);
			else if (l == 'j')
				signed_value = va_arg(cfg->list, intmax_t);
			else if (l == 'z')
				signed_value = va_arg(cfg->list, ssize_t);
			else if (l == 't')
				signed_value = va_arg(cfg->list, ptrdiff_t);
			else
				signed_value = va_arg(cfg->list, int);
			if (l == 'h' && spec->length[1] == 'h')
				signed_value = (signed char)signed_value;
			else if (l == 'h')
				signed_value = (short)signed_value;
			negative = signed_value < 0;
			value	 = negative ? (uintmax_t)(-(signed_value + 1)) + 1
								: (uintmax_t)signed_value;
		}
		else
		{
			if (l == 'l' && spec->length[1] == 'l')
				value = va_arg(cfg->list, unsigned long long);
			else if (l == 'l')
				value = va_arg(cfg->list, unsigned long);
			else if (l == 'j')
				value = va_arg(cfg->list, uintmax_t);
			else if (l == 'z')
				value = va_arg(cfg->list, size_t);
			else if (l == 't')
				value = va_arg(cfg->list, size_t);
			else
				value = va_arg(cfg->list, unsigned int);
			if (l == 'h' && spec->length[1] == 'h')
				value = (unsigned char)value;
			else if (l == 'h')
				value = (unsigned short)value;
		}
		return (render_integer(cfg, spec, value, negative));
	}
	if (strchr("fFeEgGaA", c))
	{
		if (l == 'L')
			RENDER_TYPED(long double);
		RENDER_TYPED(double);
	}
	if ((c == 'c' || c == 's') && l == 0)
		return (render_text(cfg, spec));
	if (c == 'c')
	{
		if (l == 'l')
			RENDER_TYPED(wint_t);
		RENDER_TYPED(int);
	}
	if (c == 's')
	{
		if (l == 'l')
			RENDER_TYPED(wchar_t *);
		RENDER_TYPED(char *);
	}
	if (c == 'p')
		RENDER_TYPED(void *);
	return (EINVAL);
}
#undef RENDER_TYPED

/** @brief Resolve a hashed tag and transfer callback output. */
static int	render_custom(
	t_genesis_format_	*cfg
)
{
	const char				*start	   = cfg->fm + cfg->cursor + 1;
	const char				*end	   = strchr(start, '}');
	const char				*colon;
	t_format_callback		 callback;
	char					*options;
	char					*piece;
	size_t					 length;
	int						 status;

	if (!end)
		return (EINVAL);
	colon  = memchr(start, ':', (size_t)(end - start));
	length = (size_t)((colon ? colon : end) - start);
	if (length == 0)
		return (EINVAL);
	callback = format_registry_find(start, length);
	if (!callback)
		return (EINVAL);
	length	= (size_t)(end - (colon ? colon + 1 : end));
	if (length == SIZE_MAX)
		return (EOVERFLOW);
	options = mem_alloc(length + 1);
	if (!options)
		return (errno);
	memcpy(options, colon ? colon + 1 : end, length);
	options[length] = '\0';
	errno			= 0;
	piece			= callback(va_arg(cfg->list, void *), options);
	mem_free(options);
	if (!piece)
		return (errno ? errno : EINVAL);
	status = emit(cfg, piece, strlen(piece));
	mem_free(piece);
	cfg->cursor = (size_t)(end - cfg->fm) + 1;
	return (status);
}

/* ----| Public     |----- */
/** @brief Scan the complete input using the central state. */
int	genesis_format_(
	t_genesis_format_	*cfg
)
{
	int	status;

	genesis_format_builtins_init();

	while (cfg->fm[cfg->cursor])
	{
		if (cfg->fm[cfg->cursor] != '%')
		{
			status = emit(cfg, cfg->fm + cfg->cursor, 1);
			++cfg->cursor;
		}
		else if (cfg->fm[++cfg->cursor] == '{')
			status = render_custom(cfg);
		else
		{
			status = parse_spec(cfg, &cfg->current);
			if (!status)
				status = render_standard(cfg, &cfg->current);
		}
		if (status)
			return (status);
	}
	if (!cfg->fixed_capacity && !cfg->buffer)
		return (emit(cfg, "", 0));
	return (0);
}
