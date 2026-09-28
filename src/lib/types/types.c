/**
 * 
*/
/* ----| Headers    |----- */
	/* Standard */
		//...

	/* Internal */
#include "types.h"
#include "_internal_/map/_map.h"

	/* External */
#include "../memory.h"
#include <errno.h>
#include <stdint.h>

/* ----| Prototypes |----- */
	//...

/* ----| Internals  |----- */
	//...

/* ----| Public     |----- */

size_t	hash_fnv1a(
	const char *str
)
{
	size_t	hash = 2166136261ULL;

	while (*str)
	{
		hash ^= (unsigned char)*str++;
		hash *= 16777619ULL;
	}
	return (hash);
}

t_types_settings	g_types_settings = {
	.error = {0},
	.alloc = mem_alloc,
	.realloc = mem_realloc,
	.free = mem_free,
	.dup = mem_dup,
	.hash_string = hash_fnv1a
};

int	hash_map_setup(
	t_hash_map *const	map,
	const size_t		size
)
{
	if (unlikely(!map || size == 0))
		return_error(error_invalid_arg, NULL, error_invalid_arg);
	if (map->map || map->capacity || map->length)
		return_error(error_invalid_arg, NULL, error_invalid_arg);
	if (size > SIZE_MAX / sizeof(t_hash_map_entry *))
		return_error(EOVERFLOW, NULL, EOVERFLOW);
	map->capacity = size;
	if (_hash_map_setup(map) != 0)
	{
		map->capacity = 0;
		return (setting->error.errnum);
	}
	return (0);
}

int	hash_map_destroy(
	t_hash_map *const map
)
{
	if (unlikely(!map))
		return_error(error_invalid_arg, NULL, error_invalid_arg);

	return (_hash_map_destroy(map));
}

int	hash_map_add(
	t_hash_map *const	map,
	char *const			key,
	void *const			value
)
{
	if (unlikely(!map || !map->map || !map->capacity || !key))
		return_error(error_invalid_arg, key, error_invalid_arg);

	return (_hash_map_add(map, key, value));
}

void	*hash_map_find(
	t_hash_map *const	map,
	char *const			key
)
{
	if (unlikely(!map || !map->map || !map->capacity || !key))
		return_error(error_invalid_arg, key, NULL);

	return (_hash_map_find(map, key));
}

/**
 * @brief	return a pointer to the value at the key index
*/
void	**hash_map_find_ptr(
	t_hash_map *const	map,
	char *const			key
)
{
	if (unlikely(!map || !map->map || !map->capacity || !key))
		return_error(error_invalid_arg, key, NULL);

	return (_hash_map_find_ptr(map, key));
}

void	hash_map_remove(
	t_hash_map *const	map,
	char *const			key
)
{
	if (unlikely(!map || !map->map || !map->capacity || !key))
	{
		setting->error.errnum = error_invalid_arg;
		setting->error.key = key;
		return ;
	}

	_hash_map_remove(map, key);
}

bool	hash_map_contains(
	const t_hash_map *const	map,
	const char *const		key
)
{
	if (unlikely(!map || !map->map || !map->capacity || !key))
		return_error(error_invalid_arg, key, false);

	return (_hash_map_contains(map, key));
}

int	hash_map_replace(
	t_hash_map *const	map,
	const char *const	key,
	void *const			value
)
{
	if (unlikely(!map || !map->map || !map->capacity || !key))
		return_error(error_invalid_arg, key, error_invalid_arg);

	return (_hash_map_replace(map, key, value));
}

/**
 * this function will have multiple known bucket size that reduce colition,
 * if `size` == -1, it will use the new bigger bucket size has the new size
*/
int	hash_map_resize(
	t_hash_map *const	map,
	const ssize_t		size
)
{
	if (unlikely(!map || !map->map || !map->capacity || size == 0 || size < -1))
		return_error(error_invalid_arg, NULL, error_invalid_arg);

	return (_hash_map_resize(map, size));
}
