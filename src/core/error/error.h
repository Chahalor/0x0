/**
 * @file error.h
 * @brief Public error messages and central dispatch for Genesis core.
 * @details Dispatches module-specific codes to setting, allocator and buffer.
 */
#ifndef GENESIS_CORE_ERROR_H
# define GENESIS_CORE_ERROR_H
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
 * @brief Describe an error from the core error module.
 * @param code Zero, positive errno, or negative CORE_ERROR code.
 * @return Borrowed message, including a fallback for unknown codes.
 * @par Example
 * @code{.c}
 * puts(core_error_strerror(-CORE_ERROR_UNKNOWN));
 * @endcode
 */
const char	*core_error_strerror(
	const int	code
);

/**
 * @brief Dispatch an error code to its module's message function.
 * @param module Module that returned the code.
 * @param code Zero, positive errno, or a negative module error.
 * @return Borrowed message; an invalid module has a fallback message.
 * @par Example
 * @code{.c}
 * puts(genesis_strerror(GENESIS_MODULE_BUFFER, status));
 * @endcode
 */
const char	*genesis_strerror(
	const t_genesis_core_module	module,
	const int					code
);

#endif
