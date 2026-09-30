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
// # include "../setting/types.h"

/* ************************************************************************** */
/*                                 Typedefs                                   */
/* ************************************************************************** */

/* -----| Enums     |----- */
typedef enum e_genesis_core_buffer_error	t_genesis_core_buffer_error;
typedef enum e_genesis_core_buffer_growth	t_genesis_core_buffer_growth;

/* -----| Structs   |----- */
typedef struct s_genesis_core_buffer			t_genesis_core_buffer;
typedef struct s_genesis_core_buffer_settings	t_genesis_core_buffer_settings;

/* ************************************************************************** */
/*                                 Enums                                      */
/* ************************************************************************** */

enum e_genesis_core_buffer_error
{
	BUFFER_ERROR_SUCCESS				= 0,
	BUFFER_ERROR_SOURCE_OUT_OF_RANGE	= 1,
	BUFFER_ERROR_UNKNOWN				= 2
};

enum e_genesis_core_buffer_growth
{
	BUFFER_GROWTH_GEOMETRIC	= 0,
	BUFFER_GROWTH_EXACT		= 1
};

/* ************************************************************************** */
/*                                 Unions                                     */
/* ************************************************************************** */

/* ************************************************************************** */
/*                                 Structs                                    */
/* ************************************************************************** */

/** @brief Growth policy captured when a buffer is initialized. */
struct s_genesis_core_buffer_settings
{
	size_t							initial_capacity;
	t_genesis_core_buffer_growth	growth;
};

/**
 * @brief Owned byte storage with an explicit logical length.
 * @details Fields are readable but must only be changed by the buffer API.
 * The allocator and growth policy are captured at initialization.
 */
struct s_genesis_core_buffer
{
	unsigned char	*bytes;
	size_t			length;
	size_t			capacity;
};

#endif
