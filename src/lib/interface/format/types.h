/**
 * @file types.h
 * @brief Exported callback and error types for format.
 * @details No allocator or registry internals are exposed by this header.
 */
#ifndef GENESIS_INTERFACE_FORMAT_TYPES_H
# define GENESIS_INTERFACE_FORMAT_TYPES_H
# pragma once

/* ************************************************************************** */
/*                                 Headers                                    */
/* ************************************************************************** */

/* -----| Systems   |----- */
# include <stddef.h>

/* ************************************************************************** */
/*                                 Typedefs                                   */
/* ************************************************************************** */

/**
 * @brief Convert one custom argument to an owned NUL-terminated string.
 * @details value is supplied as `(void *)` in the format call. options is
 * borrowed and valid only during the callback. Return a NUL-terminated
 * string allocated with mem_alloc(), mem_realloc(), or another function
 * returning memory managed by lib/memory. The formatter takes ownership
 * and releases it with mem_free().
 * Return NULL with errno set on failure.
 */
typedef char	*(*t_format_callback)(
	const void					*value,
	const char					*options
);

/* ************************************************************************** */
/*                                 Enums                                      */
/* ************************************************************************** */

enum e_genesis_format_error
{
	FORMAT_ERROR_SUCCESS		= 0,
	FORMAT_ERROR_INVALID_FORMAT	= 1,
	FORMAT_ERROR_UNKNOWN_TAG	= 2,
	FORMAT_ERROR_UNKNOWN		= 3
};
typedef enum e_genesis_format_error	t_genesis_format_error;

#endif
