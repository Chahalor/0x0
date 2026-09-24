/**
 * @file error.h
 * @brief Public error messages for the buffer module.
 * @details Describes standard errno and buffer-specific codes.
 */
#ifndef GENESIS_CORE_BUFFER_ERROR_H
# define GENESIS_CORE_BUFFER_ERROR_H
# pragma once

/* ************************************************************************** */
/*                                 Headers                                    */
/* ************************************************************************** */

/* -----| Internals |----- */
# include "types.h"

/* ************************************************************************** */
/*                                 Macros                                     */
/* ************************************************************************** */

/* ************************************************************************** */
/*                                 Prototypes                                 */
/* ************************************************************************** */

/**
 * @brief Describe a byte buffer result code.
 * @param code Zero, positive errno, or a negative BUFFER_ERROR code.
 * @return Borrowed message; unknown negative codes have a fallback message.
 * @par Example
 * @code{.c}
 * puts(core_buffer_strerror(-BUFFER_ERROR_UNKNOWN));
 * @endcode
 */
const char	*core_buffer_strerror(
	const int	code
);

#endif
