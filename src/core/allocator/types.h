/**
 * @file types.h
 * @brief Public allocator descriptor for core modules.
 * @details Defines the callbacks shared by allocator, setting and buffer.
 */
#ifndef GENESIS_CORE_ALLOCATOR_TYPES_H
# define GENESIS_CORE_ALLOCATOR_TYPES_H
# pragma once

/* ************************************************************************** */
/*                                 Headers                                    */
/* ************************************************************************** */

/* -----| Systems   |----- */
# include <stddef.h>

/* ************************************************************************** */
/*                                 Typedefs                                   */
/* ************************************************************************** */

/* -----| Structs   |----- */
typedef struct s_genesis_core_allocator	t_genesis_core_allocator;

/* ************************************************************************** */
/*                                 Enums                                      */
/* ************************************************************************** */

enum e_genesis_core_allocator_error
{
	ALLOCATOR_ERROR_SUCCESS		= 0,
	ALLOCATOR_ERROR_UNKNOWN		= 1
};
typedef enum e_genesis_core_allocator_error	t_genesis_core_allocator_error;

/* ************************************************************************** */
/*                                 Structs                                    */
/* ************************************************************************** */

/**
 * @brief Allocation callbacks.
 * @details alloc and free are required; realloc is optional. Allocations
 * have normal malloc alignment.
 * A failed realloc callback must preserve the original pointer.
 */
struct s_genesis_core_allocator
{
	void	*(*alloc)(
		size_t		size
	);
	void	*(*realloc)(
		void		*ptr,
		size_t		old_size,
		size_t		new_size
	);
	void	(*free)(
		void		*ptr
	);
};

#endif
