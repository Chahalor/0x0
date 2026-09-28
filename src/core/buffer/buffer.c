/**
 * @file buffer.c
 * @brief Public validation and dispatch for byte buffers.
 */

/* ----| Headers    |----- */
	/* Standard */
# include <errno.h>
# include <stdint.h>

	/* Internal */
# include "buffer.h"
# include "_internal_/_buffer.h"

	/* External */
# include "../allocator/allocator.h"

/* ----| Prototypes |----- */

/* ----| Internals  |----- */

/* ----| Public     |----- */
int	core_buffer_init(
	t_genesis_core_buffer	*const	buffer,
	const size_t					capacity
)
{
	if (!buffer)
		return (EINVAL);

	return (_buffer_init_default(buffer, capacity));
}

int	core_buffer_init_with_allocator(
	t_genesis_core_buffer			*const	buffer,
	const size_t							capacity,
	const t_genesis_core_allocator	*const	allocator
)
{
	if (!buffer || core_allocator_validate(allocator) != 0)
		return (EINVAL);

	return (_buffer_init_with_allocator(buffer, capacity, allocator));
}

int	core_buffer_init_with_settings(
	t_genesis_core_buffer		*const	buffer,
	const size_t						capacity,
	const t_genesis_settings	*const	settings
)
{
	if (!buffer || !settings)
		return (EINVAL);

	return (_buffer_init_with_settings(buffer, capacity, settings));
}

void	core_buffer_deinit(
	t_genesis_core_buffer	*const	buffer
)
{
	if (!buffer)
		return;

	_buffer_deinit(buffer);
}

t_genesis_core_buffer	*core_buffer_new(
	const size_t	capacity
)
{
	return (_buffer_new_default(capacity));
}

t_genesis_core_buffer	*core_buffer_new_with_allocator(
	const size_t							capacity,
	const t_genesis_core_allocator	*const	allocator
)
{
	if (core_allocator_validate(allocator) != 0)
	{
		errno = EINVAL;
		return (NULL);
	}

	return (_buffer_new_with_allocator(capacity, allocator));
}

t_genesis_core_buffer	*core_buffer_new_with_settings(
	const size_t						capacity,
	const t_genesis_settings	*const	settings
)
{
	if (!settings)
	{
		errno = EINVAL;
		return (NULL);
	}

	return (_buffer_new_with_settings(capacity, settings));
}

void	core_buffer_free(
	t_genesis_core_buffer	*const	buffer
)
{
	if (!buffer)
		return;

	_buffer_free(buffer);
}

int	core_buffer_reserve(
	t_genesis_core_buffer	*const	buffer,
	const size_t					min_capacity
)
{
	if (!_buffer_is_valid(buffer))
		return (EINVAL);

	return (_buffer_reserve(buffer, min_capacity));
}

int	core_buffer_resize(
	t_genesis_core_buffer	*const	buffer,
	const size_t					new_length
)
{
	if (!_buffer_is_valid(buffer))
		return (EINVAL);

	return (_buffer_resize(buffer, new_length));
}

int	core_buffer_append(
	t_genesis_core_buffer	*const	buffer,
	const void				*const	data,
	const size_t					length
)
{
	if (!_buffer_is_valid(buffer) || (!data && length != 0))
		return (EINVAL);

	if (length > SIZE_MAX - buffer->length)
		return (EOVERFLOW);

	return (_buffer_append(buffer, data, length));
}

int	core_buffer_append_fixed(
	t_genesis_core_buffer	*const	buffer,
	const void				*const	data,
	const size_t					length
)
{
	if (!_buffer_is_valid(buffer) || (!data && length != 0))
		return (EINVAL);

	if (length > SIZE_MAX - buffer->length)
		return (EOVERFLOW);

	return (_buffer_append_fixed(buffer, data, length));
}

void	core_buffer_clear(
	t_genesis_core_buffer	*const	buffer
)
{
	if (!buffer)
		return;

	_buffer_clear(buffer);
}
