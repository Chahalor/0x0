#ifndef GENESIS_MEMORY_INTERNAL_MEMORY_H
# define GENESIS_MEMORY_INTERNAL_MEMORY_H

// Header

# pragma once

/* ************************************************************************** */
/*                                 Headers                                    */
/* ************************************************************************** */

/* -----| Systems   |----- */
#include <stdlib.h>

/* -----| Globals   |----- */
# include "lib_config.h"
# include "lib_standards.h"

/* -----| Internals |----- */
# include "_types.h"
# include "../../memory.h"

/* -----| Modules   |----- */
	//...

/* ************************************************************************** */
/*                                 Macros                                     */
/* ************************************************************************** */
	//...

/* ************************************************************************** */
/*                                 Prototypes                                 */
/* ************************************************************************** */

/* -----| rb tree |----- */

#if MEM_MANAGER_TYPE == 1

rb_tree	*rb_init(void);

rb_node	*rb_create_node(
			rb_tree *T,
			size_t size
			);

void	rb_insert(
			rb_tree *T,
			rb_node *z
			);

rb_node	*rb_search(
			rb_tree *T,
			void *key
			);

void	rb_delete(
			rb_tree *T,
			rb_node *z,
			const int _free
			);

#endif

void	*mem_manager_(
	const int	_access,
	const void	*ptr,
	size_t		size
);

t_mem_manager	*mem_manager_instance_(void);
size_t			mem_manager_size_(const void *ptr);
void			mem_arena_release_all_(t_mem_manager *manager);
MEM_ARENA		*arena_new_(void);
void			arena_free_(MEM_ARENA *arena);
void			arena_reset_(MEM_ARENA *arena);
void			*arena_alloc_(MEM_ARENA *arena, size_t size);
void			*arena_dup_(MEM_ARENA *arena, void *ptr, ssize_t size);
t_arena_mark	arena_mark_(MEM_ARENA *arena);
void			arena_rewind_(MEM_ARENA *arena, t_arena_mark mark);
void			arena_trim_(MEM_ARENA *arena);

#endif	// GENESIS_MEMORY_INTERNAL_MEMORY_H
