/**
 * @file error.h
 * @brief UUID error descriptions and shared Genesis error dispatch.
 */
#ifndef GENESIS_TYPES_UUID_ERROR_H
# define GENESIS_TYPES_UUID_ERROR_H
# pragma once

/* ----| Internals |----- */
# include "types.h"

/* ----| Modules   |----- */
# include "../../../core/error/types.h"

/**
 * @brief Describe a UUID status code.
 * @param code Zero, positive errno, or negative UUID_ERROR code.
 * @return Borrowed message, never NULL. Unknown negatives use the generic
 * UUID error message; positive codes use the platform strerror message.
 * @par Example
 * @code{.c}
 * const char *message = genesis_uuid_strerror(-UUID_ERROR_INVALID_FORMAT);
 * @endcode
 */
const char	*genesis_uuid_strerror(
	const int	code
);

/**
 * @brief UUID descriptor for genesis_strerror().
 * @details Static process-lifetime value; callers never free it.
 * @par Example
 * @code{.c}
 * const char *message = genesis_strerror(&genesis_uuid_error_module, code);
 * @endcode
 */
extern const t_genesis_error_module	genesis_uuid_error_module;

#endif
