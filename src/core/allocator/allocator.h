/**
 * @file allocator.h
 * @brief Public allocation operations for Genesis core modules.
 * @details Uses the allocator descriptor from types.h. The setting module
 * supplies the default descriptor and may provide a custom one.
 */
#ifndef GENESIS_CORE_ALLOCATOR_H
# define GENESIS_CORE_ALLOCATOR_H
# pragma once

/* ************************************************************************** */
/*                                 Headers                                    */
/* ************************************************************************** */

/* -----| Internals |----- */
# include "types.h"

/* ************************************************************************** */
/*                                 Macros                                     */
/* ************************************************************************** */

/* ************************************************************************** */
/*                                 Prototypes                                 */
/* ************************************************************************** */

/**
 * @brief Create a descriptor backed by malloc, realloc and free.
 * @return An independent system allocator descriptor.
 * @par Example
 * @code{.c}
 * t_genesis_core_allocator allocator = core_allocator_system();
 * @endcode
 */
t_genesis_core_allocator	core_allocator_system(void);

/**
 * @brief Check that an allocator can allocate and release memory.
 * @param allocator Descriptor to inspect; realloc may be NULL.
 * @return 0 when valid, or positive EINVAL otherwise.
 * @par Example
 * @code{.c}
 * if (core_allocator_validate(&allocator) != 0) return (EINVAL);
 * @endcode
 */
int	core_allocator_validate(
	const t_genesis_core_allocator	*const	allocator
);

/**
 * @brief Allocate a nonzero number of bytes.
 * @param allocator Descriptor to use for this allocation.
 * @param size Number of bytes requested.
 * @return Owned pointer or NULL; errno is EINVAL or ENOMEM on failure.
 * @par Example
 * @code{.c}
 * void *data = core_alloc(&allocator, 64);
 * @endcode
 */
void	*core_alloc(
	const t_genesis_core_allocator	*const	allocator,
	const size_t							size
);

/**
 * @brief Allocate an array after checking multiplication overflow.
 * @param allocator Descriptor to use.
 * @param count Number of elements; must be nonzero.
 * @param element_size Bytes per element; must be nonzero.
 * @return Owned pointer or NULL; errno is EINVAL, EOVERFLOW or ENOMEM.
 * @par Example
 * @code{.c}
 * int *values = core_alloc_array(&allocator, 8, sizeof(int));
 * @endcode
 */
void	*core_alloc_array(
	const t_genesis_core_allocator	*const	allocator,
	const size_t							count,
	const size_t							element_size
);

/**
 * @brief Allocate an array and zero all of its bytes.
 * @param allocator Descriptor to use.
 * @param count Number of elements; must be nonzero.
 * @param element_size Bytes per element; must be nonzero.
 * @return Owned zeroed pointer or NULL; errno is EINVAL, EOVERFLOW or ENOMEM.
 * @par Example
 * @code{.c}
 * int *values = core_calloc(&allocator, 8, sizeof(int));
 * @endcode
 */
void	*core_calloc(
	const t_genesis_core_allocator	*const	allocator,
	const size_t							count,
	const size_t							element_size
);

/**
 * @brief Resize an owned allocation, preserving data on failure.
 * @param allocator Descriptor that allocated ptr.
 * @param ptr Existing allocation, or NULL to allocate a new block.
 * @param old_size Size of the old allocation in bytes.
 * @param new_size New size in bytes; zero releases a non-NULL ptr.
 * @return New owned pointer or NULL. On allocation failure, ptr stays valid;
 * errno is EINVAL or ENOMEM. An absent callback uses allocate, copy and free.
 * @par Example
 * @code{.c}
 * void *grown = core_realloc(&allocator, data, 64, 128);
 * if (grown) data = grown;
 * @endcode
 */
void	*core_realloc(
	const t_genesis_core_allocator	*const	allocator,
	void							*const	ptr,
	const size_t							old_size,
	const size_t							new_size
);

/**
 * @brief Release memory with the allocator that created it.
 * @param allocator Descriptor that allocated ptr.
 * @param ptr Pointer to release; NULL is accepted.
 * @return Nothing. Invalid non-NULL requests set errno to EINVAL.
 * @par Example
 * @code{.c}
 * core_free(&allocator, data);
 * @endcode
 */
void	core_free(
	const t_genesis_core_allocator	*const	allocator,
	void							*const	ptr
);

#endif
