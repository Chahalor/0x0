/**
 * @file memory.h
 * @brief API public to the memory manager
 *
 * this module provide many fonction to manager more safely the memory.
 * Internal storage uses the current global callbacks from core/setting for
 * each allocation and release. Replace incompatible callbacks only after
 * releasing live memory.
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

// remove a pointer from the manager, does not free any memory
void	mem_manager_rm(
	void	*const	ptr
);

/**
 * @brief Create a fixed-capacity arena owned by the memory manager.
 * @return Arena with 64 KiB of storage, or NULL with errno ENOMEM or EINVAL.
 * @par Example
 * @code{.c}
 * MEM_ARENA *arena = mem_arena_new();
 * mem_arena_free(arena);
 * @endcode
 */
MEM_ARENA	*mem_arena_new(void);

/**
 * @brief Release an arena and all its allocations.
 * @param arena Arena returned by mem_arena_new(), or NULL for a no-op.
 * @return Nothing. Allocations and marks become invalid. Unknown arenas set
 * errno to EINVAL.
 * @par Example
 * @code{.c}
 * mem_arena_free(arena);
 * @endcode
 */
void		mem_arena_free(
	MEM_ARENA	*const	arena
);

/**
 * @brief Invalidate all allocations and marks while retaining storage.
 * @param arena Live arena, or NULL for a no-op.
 * @return Nothing. Unknown arenas set errno to EINVAL.
 * @par Example
 * @code{.c}
 * mem_arena_reset(arena);
 * @endcode
 */
void	mem_arena_reset(
	MEM_ARENA	*const	arena
);

/**
 * @brief Allocate zeroed bytes with max_align_t alignment in an arena.
 * @param arena Live arena; must not be NULL.
 * @param size Positive byte count.
 * @return Pointer owned by the arena, or NULL with errno EINVAL for invalid
 * input, ENOMEM when the fixed capacity is exhausted, or EOVERFLOW.
 * @note Individual blocks cannot be passed to mem_free().
 * @par Example
 * @code{.c}
 * int *values = mem_arena_alloc(arena, 8 * sizeof(*values));
 * @endcode
 */
void	*mem_arena_alloc(
	MEM_ARENA	*const	arena,
	const size_t		size
);

/**
 * @brief Copy bytes into a new allocation owned by an arena.
 * @param arena Live arena; must not be NULL.
 * @param ptr Readable source; must not be NULL.
 * @param size Positive byte count, or -1 for the size of a mem_alloc block.
 * @return Copied bytes, or NULL with errno EINVAL, ENOMEM or EOVERFLOW.
 * @par Example
 * @code{.c}
 * char *copy = mem_arena_dup(arena, source, length);
 * @endcode
 */
void	*mem_arena_dup(
	MEM_ARENA	*const	arena,
	void		*const	ptr,
	const ssize_t		size
);

/**
 * @brief Save the current allocation position as an arena-owned mark.
 * @param arena Live arena; must not be NULL.
 * @return Opaque mark, or NULL with errno EINVAL, ENOMEM or EOVERFLOW. Marks
 * live until reset, rewind past them, or arena release.
 * @par Example
 * @code{.c}
 * t_arena_mark mark = mem_arena_mark(arena);
 * @endcode
 */
t_arena_mark	mem_arena_mark(
	MEM_ARENA	*const	arena
);

/**
 * @brief Invalidate only allocations made after a mark from this arena.
 * @details Earlier allocation bytes and pointers remain intact. Marks created
 * after the target become invalid. Invalid marks set errno to EINVAL.
 * @param arena Live arena; must not be NULL.
 * @param mark Mark previously returned for this arena.
 * @return Nothing. The target mark remains valid for repeated rewinds.
 * @par Example
 * @code{.c}
 * mem_arena_rewind(arena, mark);
 * @endcode
 */
void	mem_arena_rewind(
	MEM_ARENA	*const	arena,
	const t_arena_mark	mark
);

/**
 * @brief Release backing storage when the arena is empty.
 * @param arena Live arena, or NULL for a no-op.
 * @return Nothing. Live pointers never move; a later allocation restores
 * storage. Unknown arenas set errno to EINVAL.
 * @par Example
 * @code{.c}
 * mem_arena_reset(arena);
 * mem_arena_trim(arena);
 * @endcode
 */
void	mem_arena_trim(
	MEM_ARENA	*const	arena
);

#endif	// GENESIS_MEMORY_H
