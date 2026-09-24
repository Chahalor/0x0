/**
 * @file error.h
 * @brief Public error messages for the setting module.
 * @details Describes standard errno and setting-specific codes.
 */
#ifndef GENESIS_CORE_SETTING_ERROR_H
# define GENESIS_CORE_SETTING_ERROR_H
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
 * @brief Describe a settings result code.
 * @param code Zero, positive errno, or a negative SETTING_ERROR code.
 * @return Borrowed message; unknown negative codes have a fallback message.
 * @par Example
 * @code{.c}
 * puts(core_setting_strerror(-SETTING_ERROR_UNKNOWN));
 * @endcode
 */
const char	*core_setting_strerror(
	const int	code
);

#endif
