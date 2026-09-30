/**
 * @file error.h
 * @brief Public error messages for the format module.
 * @details Depends on core error dispatch and the format error enum.
 */
#ifndef GENESIS_INTERFACE_FORMAT_ERROR_H
# define GENESIS_INTERFACE_FORMAT_ERROR_H
# pragma once

/* -----| Internals |----- */
# include "types.h"

/* -----| Modules   |----- */
# include "../../../core/error/types.h"

/**
 * @brief Describe a format status code.
 * @param code Zero, a positive standard errno, or a negative FORMAT_ERROR.
 * @return Borrowed static or libc message, never NULL; do not free it.
 * Unknown negative codes use the FORMAT_ERROR_UNKNOWN message.
 * @par Example
 * @code{.c}
 * const char *text = genesis_format_strerror(-FORMAT_ERROR_UNKNOWN_TAG);
 * @endcode
 */
const char	*genesis_format_strerror(
	int	code
);

/**
 * @brief Descriptor for genesis_strerror() dispatch.
 * @details Stable for process lifetime; callers borrow it and never free it.
 * @par Example
 * @code{.c}
 * const char *text = genesis_strerror(&genesis_format_error_module, code);
 * @endcode
 */
extern const t_genesis_error_module	genesis_format_error_module;

#endif
