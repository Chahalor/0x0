/**
 * @file allocator.c
 * @brief Public allocator validation and dispatch.
 */

/* ----| Headers    |----- */
	/* Standard */
# include <errno.h>
# include <stdint.h>

	/* Internal */
# include "allocator.h"
# include "_internal_/_allocator.h"

/* ----| Prototypes |----- */

/* ----| Internals  |----- */

/* ----| Public     |----- */
t_genesis_core_allocator	core_allocator_system(void)
{
	return (_allocator_system());
}

int	core_allocator_validate(
	const t_genesis_core_allocator	*const	allocator
)
{
	return (_allocator_validate(allocator));
}

void	*core_alloc(
	const t_genesis_core_allocator	*const	allocator,
	const size_t							size
)
{
	if (_allocator_validate(allocator) != 0 || size == 0)
	{
		errno = EINVAL;
		return (NULL);
	}

	return (_allocator_alloc(allocator, size));
}

void	*core_alloc_array(
	const t_genesis_core_allocator	*const	allocator,
	const size_t							count,
	const size_t							element_size
)
{
	if (_allocator_validate(allocator) != 0
		|| count == 0 || element_size == 0)
	{
		errno = EINVAL;
		return (NULL);
	}

	if (count > SIZE_MAX / element_size)
	{
		errno = EOVERFLOW;
		return (NULL);
	}

	return (_allocator_alloc(allocator, count * element_size));
}

void	*core_calloc(
	const t_genesis_core_allocator	*const	allocator,
	const size_t							count,
	const size_t							element_size
)
{
	if (_allocator_validate(allocator) != 0
		|| count == 0 || element_size == 0)
	{
		errno = EINVAL;
		return (NULL);
	}

	if (count > SIZE_MAX / element_size)
	{
		errno = EOVERFLOW;
		return (NULL);
	}

	return (_allocator_calloc(allocator, count, element_size));
}

void	*core_realloc(
	const t_genesis_core_allocator	*const	allocator,
	void							*const	ptr,
	const size_t							old_size,
	const size_t							new_size
)
{
	if (_allocator_validate(allocator) != 0 || (!ptr && new_size == 0))
	{
		errno = EINVAL;
		return (NULL);
	}

	return (_allocator_realloc(allocator, ptr, old_size, new_size));
}

void	core_free(
	const t_genesis_core_allocator	*const	allocator,
	void							*const	ptr,
	const size_t							size
)
{
	if (!ptr)
		return;

	if (_allocator_validate(allocator) != 0)
	{
		errno = EINVAL;
		return;
	}

	_allocator_free(allocator, ptr, size);
}
