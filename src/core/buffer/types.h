/**
 * @file types.h
 * @brief Public byte buffer type for Genesis core.
 * @details The buffer owns its bytes and retains its allocator descriptor.
 */
#ifndef GENESIS_CORE_BUFFER_TYPES_H
# define GENESIS_CORE_BUFFER_TYPES_H
# pragma once

/* ************************************************************************** */
/*                                 Headers                                    */
/* ************************************************************************** */

/* -----| Systems   |----- */
# include <stddef.h>

/* -----| Modules   |----- */
# include "../allocator/types.h"

/* ************************************************************************** */
/*                                 Typedefs                                   */
/* ************************************************************************** */

/* -----| Enums     |----- */
typedef enum e_genesis_core_buffer_error	t_genesis_core_buffer_error;

/* -----| Structs   |----- */
typedef struct s_genesis_core_buffer	t_genesis_core_buffer;

/* ************************************************************************** */
/*                                 Enums                                      */
/* ************************************************************************** */

enum e_genesis_core_buffer_error
{
	BUFFER_ERROR_SUCCESS = 0,
	BUFFER_ERROR_SOURCE_OUT_OF_RANGE = 1,
	BUFFER_ERROR_UNKNOWN = 2
};

/* ************************************************************************** */
/*                                 Unions                                     */
/* ************************************************************************** */

/* ************************************************************************** */
/*                                 Structs                                    */
/* ************************************************************************** */

/**
 * @brief Owned byte storage with an explicit logical length.
 * @details Fields are readable but must only be changed by the buffer API.
 */
struct s_genesis_core_buffer
{
	unsigned char				*bytes;
	size_t						length;
	size_t						capacity;
	t_genesis_core_allocator	allocator;
};

#endif
