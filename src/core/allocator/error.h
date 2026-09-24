/**
 * @file error.h
 * @brief Public error messages for the allocator module.
 * @details Describes standard errno and allocator-specific codes.
 */
#ifndef GENESIS_CORE_ALLOCATOR_ERROR_H
# define GENESIS_CORE_ALLOCATOR_ERROR_H
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
 * @brief Describe a allocation result code.
 * @param code Zero, positive errno, or a negative ALLOCATOR_ERROR code.
 * @return Borrowed message; unknown negative codes have a fallback message.
 * @par Example
 * @code{.c}
 * puts(core_allocator_strerror(-ALLOCATOR_ERROR_UNKNOWN));
 * @endcode
 */
const char	*core_allocator_strerror(
	const int	code
);

#endif
