/**
 * @file format.h
 * @brief Public in-memory formatting API.
 * @details Formatting performs no file or terminal IO. Dynamic results are
 * managed by lib/memory and must be released with mem_free(). Registry and
 * settings access require external synchronization when changed concurrently.
 * Conversions run left to right: prior `%n` writes and callback side effects
 * remain if a later conversion fails.
 */
#ifndef GENESIS_INTERFACE_FORMAT_H
# define GENESIS_INTERFACE_FORMAT_H
# pragma once

/* ************************************************************************** */
/*                                 Headers                                    */
/* ************************************************************************** */

/* -----| Systems   |----- */
# include <stdarg.h>
# include <stddef.h>

/* -----| Internals |----- */
# include "format/types.h"

/* -----| Modules   |----- */
# include "../memory.h"

/* ************************************************************************** */
/*                                 MACRO                                      */
/* ************************************************************************** */

# define	GENESIS_FORMAT_BLTN_COLOR	"COLOR"
# define	GENESIS_FORMAT_UUID			"UUID"
# define	GENESIS_FORMAT_JSON			"JSON"

/* ************************************************************************** */
/*                                 Prototypes                                 */
/* ************************************************************************** */

/**
 * @brief Format arguments into a newly allocated C string.
 * @param format Non-NULL printf-style format. A custom tag uses
 * `%{TAG:options}`. Standard conversions in options are expanded first,
 * consuming their arguments left to right; the tag then consumes one
 * `(void *)` value. Use `%%` for a literal percent in options. `%n` writes
 * its counter. Positional arguments are unsupported.
 * @return Owned string, including for empty input, or NULL with errno set to
 * EINVAL, ENOMEM, or EOVERFLOW. No result remains on error, though earlier
 * `%n` writes and callback side effects can remain. The last applied
 * output limit is captured at call time. Release with mem_free().
 * @par Example
 * @code{.c}
 * char *text = genesis_format("%s %d", "value", 7);
 * mem_free(text);
 * @endcode
 */
char	*genesis_format(
	const char	*format,
	...
);

/**
 * @brief va_list form of genesis_format().
 * @param format Non-NULL format with matching arguments.
 * @param args Borrowed argument list; copied without consuming the caller's
 * list.
 * @return Owned string or NULL with errno set to EINVAL, ENOMEM, or
 * EOVERFLOW. No result remains on error; earlier `%n` writes and callback
 * side effects can remain. Release with mem_free().
 * @par Example
 * @code{.c}
 * char *text = genesis_vformat(format, args);
 * mem_free(text);
 * @endcode
 */
char	*genesis_vformat(
	const char	*format,
	va_list		args
);

/**
 * @brief Format into a caller-owned fixed buffer.
 * @param output Writable buffer when size is nonzero; NULL is valid for zero.
 * @param size Capacity in bytes including the terminating NUL.
 * @param format Non-NULL format with matching arguments.
 * @return Full byte count excluding NUL, even if truncated, or SIZE_MAX with
 * errno EINVAL, ENOMEM, or EOVERFLOW. The applied format output limit applies
 * to the full count. A nonzero output is NUL-terminated on success and emptied on
 * failure. Earlier `%n` writes and callback side effects can remain. The
 * caller retains buffer ownership.
 * @par Example
 * @code{.c}
 * char text[4];
 * size_t needed = genesis_sformat(text, sizeof(text), "%s", "hello");
 * @endcode
 */
size_t	genesis_sformat(
	char		*restrict	output,
	size_t					size,
	const char	*restrict	format,
	...
);

/**
 * @brief va_list form of genesis_sformat().
 * @param output Writable buffer when size is nonzero; NULL for zero.
 * @param size Capacity including NUL.
 * @param format Non-NULL format with matching arguments.
 * @param args Borrowed list, copied without consuming the caller's list.
 * @return Full byte count or SIZE_MAX with errno set on failure. Nonzero
 * output is terminated on success and emptied on failure; ownership stays
 * with the caller. Earlier `%n` writes and callback side effects can remain.
 * @par Example
 * @code{.c}
 * size_t needed = genesis_vsformat(NULL, 0, format, args);
 * @endcode
 */
size_t	genesis_vsformat(
	char		*restrict	output,
	size_t					size,
	const char	*restrict	format,
	va_list					args
);

/**
 * @brief Register or replace a custom tag.
 * @param tag Nonempty name without braces or colons.
 * @param callback Non-NULL callback consuming one `(void *)` format argument.
 * It receives borrowed options and transfers ownership of its string result
 * to the formatter. Allocate that result with mem_alloc() or mem_realloc().
 * A replacement keeps the old storage.
 * @return 0, EINVAL, ENOMEM, or EOVERFLOW. Existing tags remain unchanged
 * on failure. Registry changes require external synchronization.
 * @par Example
 * @code{.c}
 * genesis_format_set("JSON", json_format);
 * @endcode
 */
int	genesis_format_set(
	const char			*tag,
	t_format_callback	callback
);

/**
 * @brief Remove a registered custom tag.
 * @param tag Nonempty name without braces or colons.
 * @return 0, EINVAL for invalid names, or -FORMAT_ERROR_UNKNOWN_TAG when
 * absent. On success the captured allocator releases the tag storage.
 * Registry changes require external synchronization.
 * @par Example
 * @code{.c}
 * genesis_format_rm("JSON");
 * @endcode
 */
int	genesis_format_rm(
	const char	*tag
);

#endif
