/**
 * @file _buffer.h
 * @brief Private byte buffer operations.
 * @details These declarations are restricted to the buffer module.
 */
#ifndef GENESIS_CORE_INTERNAL_BUFFER_H
# define GENESIS_CORE_INTERNAL_BUFFER_H
# pragma once

/* ************************************************************************** */
/*                                 Headers                                    */
/* ************************************************************************** */

/* -----| Internals |----- */
# include "../types.h"

/* ************************************************************************** */
/*                                 Prototypes                                 */
/* ************************************************************************** */

int						_buffer_is_valid(
	const t_genesis_core_buffer	*const	buffer
);

int						_buffer_init_default(
	t_genesis_core_buffer			*const	buffer,
	const size_t							capacity
);

void					_buffer_deinit(
	t_genesis_core_buffer			*const	buffer
);

t_genesis_core_buffer	*_buffer_new_default(
	const size_t	capacity
);

void					_buffer_free(
	t_genesis_core_buffer	*const	buffer
);

int						_buffer_reserve(
	t_genesis_core_buffer	*const	buffer,
	const size_t					min_capacity
);

int						_buffer_resize(
	t_genesis_core_buffer	*const	buffer,
	const size_t					new_length
);

int						_buffer_append(
	t_genesis_core_buffer	*const	buffer,
	const void				*const	data,
	const size_t					length
);

int						_buffer_append_fixed(
	t_genesis_core_buffer	*const	buffer,
	const void				*const	data,
	const size_t					length
);

void					_buffer_clear(
	t_genesis_core_buffer	*const	buffer
);

#endif
