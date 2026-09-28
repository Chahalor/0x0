/**
 * 
*/
#ifndef GENESIS_TYPES_H
# define GENESIS_TYPES_H
# pragma once

/* ************************************************************************** */
/*                                 Headers                                    */
/* ************************************************************************** */

/* -----| Systems   |----- */
	//...

/* -----| Globals   |----- */
# include "lib_config.h"
# include "lib_standards.h"

/* -----| Internals |----- */
# include "types/types.h"

/* -----| Modules   |----- */
# include "processor/processor.h"

/* ************************************************************************** */
/*                                 Macros                                     */
/* ************************************************************************** */

#define hash_map_foreach (var, map) \
for (uint __i = 0; __i < HASH_MAP_BUCKET_SIZE; __i++)

/* ************************************************************************** */
/*                                 Prototypes                                 */
/* ************************************************************************** */

#pragma region Array

#pragma region Hash Map

size_t	hash_fnv1a(
	const char *str
);

/**
 * @brief Initialize a zero-initialized map with a nonzero bucket count.
 * @param map Caller-owned map initialized to {0}; an active map is rejected.
 * @param size Number of buckets.
 * @return 0 on success, error_invalid_arg for invalid input, EOVERFLOW or
 * ENOMEM for allocation failure. On failure the map remains empty.
 * @par Example
 * @code{.c}
 * t_hash_map map = {0};
 * hash_map_setup(&map, 17);
 * @endcode
 */
int	hash_map_setup(
	t_hash_map *const	map,
	const size_t		size
);

/**
 * @brief Free the copied keys, entries and bucket array, then reset the map.
 * @param map Initialized or empty map. Values are borrowed and are never freed.
 * @return 0 on success; error_invalid_arg for NULL.
 * @par Example
 * @code{.c}
 * hash_map_destroy(&map);
 * @endcode
 */
int	hash_map_destroy(
	t_hash_map *const map
);

/**
 * @brief Copy a key and store a borrowed value pointer.
 * @param map Initialized map.
 * @param key Non-NULL NUL-terminated key, copied into the entry.
 * @param value Borrowed pointer, which may be NULL.
 * @return 0, EEXIST when the key is already present, or an allocation or
 * invalid-argument error. Duplicate insertion does not change the old value;
 * use hash_map_replace to update it.
 * @par Example
 * @code{.c}
 * hash_map_add(&map, "answer", &answer);
 * @endcode
 */
int	hash_map_add(
	t_hash_map *const	map,
	char *const			key,
	void *const			value
);

/**
 * @brief Find a borrowed value by key.
 * @return The stored pointer or NULL if absent or invalid. A stored NULL value
 * is indistinguishable from an absent key; use hash_map_contains in that case.
 */
void	*hash_map_find(
	t_hash_map *const	map,
	char *const			key
);

/**
 * @brief Return the address of the borrowed value slot for an existing key.
 * @return Slot address or NULL if absent or invalid. The slot is valid until
 * the entry is removed or the map is destroyed.
*/
void	**hash_map_find_ptr(
	t_hash_map *const	map,
	char *const			key
);

/**
 * @brief Remove a key and free its entry, leaving its borrowed value untouched.
 * @param map Initialized map.
 * @param key Key to remove; an absent key has no effect.
 */
void	hash_map_remove(
	t_hash_map *const	map,
	char *const			key
);

/** @brief Test whether a key exists, including one with a NULL value. */
bool	hash_map_contains(
	const t_hash_map *const	map,
	const char *const		key
);

/**
 * @brief Replace the borrowed value for an existing key.
 * @return 0 on success, -1 for an absent key, or error_invalid_arg.
 * The previous value remains owned by the caller.
 */
int hash_map_replace(
	t_hash_map *const	map,
	const char *const	key,
	void *const			value
);

/**
 * @brief Rehash entries into a new bucket array without changing key/value ownership.
 * @param map Initialized map.
 * @param size Positive bucket count; -1 grows to 2 * capacity + 1.
 * @return 0, error_invalid_arg, EOVERFLOW or ENOMEM. Failure leaves the map
 * and every entry unchanged.
*/
int	hash_map_resize(
	t_hash_map *const	map,
	const ssize_t		size
);

#endif	// GENESIS_TYPES_H
