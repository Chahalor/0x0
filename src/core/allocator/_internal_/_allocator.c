/**
 * @file _allocator.c
 * @brief Private callback dispatch and system allocator.
 */

/* ----| Headers    |----- */
	/* Standard */
# include <errno.h>
# include <stdlib.h>
# include <string.h>

	/* Internal */
# include "_allocator.h"

/* ----| Prototypes |----- */
static void	*_system_alloc(
	void		*const	context,
	const size_t		size
);
static void	*_system_realloc(
	void		*const	context,
	void		*const	ptr,
	const size_t		old_size,
	const size_t		new_size
);
static void	_system_free(
	void		*const	context,
	void		*const	ptr,
	const size_t		size
);

/* ----| Internals  |----- */
/** @brief Forward allocation to libc; context is unused. */
static void	*_system_alloc(
	void		*const	context,
	const size_t		size
)
{
	(void)context;
	return (malloc(size));
}

/** @brief Forward reallocation to libc; size metadata is unused. */
static void	*_system_realloc(
	void		*const	context,
	void		*const	ptr,
	const size_t		old_size,
	const size_t		new_size
)
{
	(void)context;
	(void)old_size;
	return (realloc(ptr, new_size));
}

/** @brief Forward deallocation to libc; size metadata is unused. */
static void	_system_free(
	void		*const	context,
	void		*const	ptr,
	const size_t		size
)
{
	(void)context;
	(void)size;
	free(ptr);
}

/** @brief Build the default descriptor without shared mutable state. */
t_genesis_core_allocator	_allocator_system(void)
{
	return ((t_genesis_core_allocator){
		.context = NULL,
		.alloc = _system_alloc,
		.realloc = _system_realloc,
		.free = _system_free
	});
}

/** @brief Check the callbacks required by every allocator operation. */
int	_allocator_validate(
	const t_genesis_core_allocator	*const	allocator
)
{
	return (allocator && allocator->alloc && allocator->free ? 0 : EINVAL);
}

/** @brief Dispatch allocation and normalize callback failure to ENOMEM. */
void	*_allocator_alloc(
	const t_genesis_core_allocator	*const	allocator,
	const size_t							size
)
{
	void	*ptr;

	ptr = allocator->alloc(allocator->context, size);
	if (!ptr)
		errno = ENOMEM;

	return (ptr);
}

/** @brief Allocate an array and clear every byte. */
void	*_allocator_calloc(
	const t_genesis_core_allocator	*const	allocator,
	const size_t							count,
	const size_t							element_size
)
{
	void	*ptr;

	ptr = _allocator_alloc(allocator, count * element_size);
	if (ptr)
		memset(ptr, 0, count * element_size);

	return (ptr);
}

/** @brief Use the callback or allocate, copy and release on growth. */
void	*_allocator_realloc(
	const t_genesis_core_allocator	*const	allocator,
	void							*const	ptr,
	const size_t							old_size,
	const size_t							new_size
)
{
	void	*replacement;

	if (!ptr)
		return (_allocator_alloc(allocator, new_size));

	if (new_size == 0)
	{
		_allocator_free(allocator, ptr, old_size);
		return (NULL);
	}

	if (allocator->realloc)
	{
		replacement = allocator->realloc(allocator->context, ptr,
				old_size, new_size);
		if (!replacement)
			errno = ENOMEM;

		return (replacement);
	}

	replacement = _allocator_alloc(allocator, new_size);
	if (!replacement)
		return (NULL);

	memcpy(replacement, ptr, old_size < new_size ? old_size : new_size);
	_allocator_free(allocator, ptr, old_size);
	return (replacement);
}

/** @brief Dispatch deallocation to the descriptor that owns the pointer. */
void	_allocator_free(
	const t_genesis_core_allocator	*const	allocator,
	void							*const	ptr,
	const size_t							size
)
{
	allocator->free(allocator->context, ptr, size);
}
