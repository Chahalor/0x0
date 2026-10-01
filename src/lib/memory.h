/**
 * @file memory.h
 * @brief API public to the memory manager
 *
 * this module provide many fonction to manager more safely the memory.
 * Internal storage uses the global callbacks from core/setting. A manager
 * keeps its allocator until mem_free_all; its next allocation captures the
 * callbacks currently installed in G_SETTINGS.
 *
 * @ingroup memory
 */
#ifndef GENESIS_MEMORY_H
# define GENESIS_MEMORY_H

# pragma once

/* ************************************************************************** */
/*                                 Headers                                    */
/* ************************************************************************** */

/* -----| Systems   |----- */
#include <stddef.h>
#include <sys/types.h>

/* -----| Globals   |----- */
# include "standards/lib_config.h"
# include "standards/lib_standards.h"

/* -----| Internals |----- */
# include "memory/types.h"

/* -----| Modules   |----- */
#include "math.h"

/* ************************************************************************** */
/*                                 Macros                                     */
/* ************************************************************************** */
	//...

/* ************************************************************************** */
/*                                 Prototypes                                 */
/* ************************************************************************** */

/**
 * @brief Allocate and register a zero-initialized block of bytes.
 * @param _size Requested byte count; must be nonzero.
 * @return Owned pointer, or NULL with errno set to EINVAL, EOVERFLOW or ENOMEM.
 * @note Release the result with mem_free or mem_free_all.
 * 
 * @example	char	*str = mem_alloc(sizeof(char) * strlen(input));
 * 
 * @version	1.1.0
 */
void	*mem_alloc(
			const size_t _size
			);

/**
 * @brief Resize a registered block, preserving its contents up to the smaller size.
 * @param _ptr Registered block, or NULL to allocate a new block.
 * @param _size New byte count. Zero releases a non-NULL block and returns NULL.
 * @return New owned pointer, or NULL with errno set to EINVAL, EOVERFLOW or
 * ENOMEM on failure. Failure leaves the original block valid. A foreign
 * pointer is rejected with EINVAL. Releasing at size zero sets errno to zero.
 * @par Example
 * @code{.c}
 * void *grown = mem_realloc(block, 128);
 * if (grown) block = grown;
 * @endcode
 * 
 * @version	1.1.0
*/
void	*mem_realloc(
			void *_ptr,
			const size_t _size
			);

/**
 * @brief Release and unregister a block.
 * @param _ptr Registered block, or NULL for a no-op. A foreign pointer is
 * ignored and sets errno to EINVAL.
 * @return Nothing.
 * @par Example
 * @code{.c}
 * mem_free(block);
 * @endcode
 * 
 * @version	1.1.0
*/
void	mem_free(
			void *const restrict _ptr
			);

/**
 * @brief Copy bytes into a new registered block.
 * @param _ptr Non-NULL source. It may be a foreign readable block when an
 * explicit byte count is supplied.
 * @param _size Positive byte count, or -1 to copy the full registered block.
 * A count exceeding a registered source block is rejected.
 * @return New owned block, or NULL with errno set to EINVAL, EOVERFLOW or ENOMEM.
 * @par Example
 * @code{.c}
 * void *copy = mem_dup(block, -1);
 * @endcode
 * 
 * @version	1.0.0
 */
void	*mem_dup(
			const void *const restrict _ptr,
			const ssize_t _size
			);

/**
 * @brief Concatenate bytes from two sources into a new registered block.
 * @param _ptr1 First non-NULL readable source.
 * @param _ptr2 Second non-NULL readable source.
 * @param _size1 Positive byte count, or -1 for the full registered block.
 * @param _size2 Positive byte count, or -1 for the full registered block.
 * Counts beyond registered block sizes are rejected; foreign sources require
 * explicit counts.
 * @return New owned block, or NULL with errno set to EINVAL, EOVERFLOW or ENOMEM.
 * @par Example
 * @code{.c}
 * void *joined = mem_join(first, second, first_size, second_size);
 * @endcode
 * 
 * @version	1.0.0
*/
void	*mem_join(
			const void *const restrict _ptr1,
			const void *const restrict _ptr2,
			const ssize_t _size1,
			const ssize_t _size2
			);

/**
 * @brief Release every registered block. Safe to call repeatedly; later
 * allocations recreate the manager. All previously returned pointers become
 * invalid.
 * @return Nothing.
 * @par Example
 * @code{.c}
 * mem_free_all();
 * @endcode
 * 
 * @version	1.0.0
*/
void	mem_free_all(void);

#endif	// GENESIS_MEMORY_H
