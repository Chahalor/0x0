/**
 * @file types.h
 * @brief Public UUID value and error types.
 * @details A UUID contains 16 bytes. The UUID alias is retained for existing
 * users; UUID is the canonical Genesis type name.
 */
#ifndef GENESIS_TYPES_UUID_TYPES_H
# define GENESIS_TYPES_UUID_TYPES_H
# pragma once

/* ----| Systems   |----- */
# include <stdint.h>

/* ----| Typedefs  |----- */
typedef struct s_genesis_uuid	UUID;

/* ----| Enums     |----- */
enum e_genesis_uuid_error
{
	UUID_ERROR_SUCCESS				= 0,
	UUID_ERROR_INVALID_FORMAT		= 1,
	UUID_ERROR_UNSUPPORTED_VERSION	= 2,
	UUID_ERROR_UNKNOWN				= 3
};
typedef enum e_genesis_uuid_error t_genesis_uuid_error;

/* ----| Structs   |----- */


#endif
