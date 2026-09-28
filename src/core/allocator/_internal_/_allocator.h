/**
 * @file _allocator.h
 * @brief Private allocator operations.
 * @details Used only by the allocator module's public wrappers.
 */
#ifndef GENESIS_CORE_INTERNAL_ALLOCATOR_H
# define GENESIS_CORE_INTERNAL_ALLOCATOR_H
# pragma once

/* ************************************************************************** */
/*                                 Headers                                    */
/* ************************************************************************** */

/* -----| Internals |----- */
# include "../types.h"

/* ************************************************************************** */
/*                                 Prototypes                                 */
/* ************************************************************************** */

t_genesis_core_allocator	_allocator_system(void);
int						_allocator_validate(
	const t_genesis_core_allocator	*const	allocator
);
void					*_allocator_alloc(
	const t_genesis_core_allocator	*const	allocator,
	const size_t							size
);
void					*_allocator_calloc(
	const t_genesis_core_allocator	*const	allocator,
	const size_t							count,
	const size_t							element_size
);
void					*_allocator_realloc(
	const t_genesis_core_allocator	*const	allocator,
	void							*const	ptr,
	const size_t							old_size,
	const size_t							new_size
);
void					_allocator_free(
	const t_genesis_core_allocator	*const	allocator,
	void							*const	ptr,
	const size_t							size
);

#endif
