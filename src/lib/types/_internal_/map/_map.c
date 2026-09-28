/**
 * 
*/
/* ----| Headers    |----- */
	/* Standard */
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>

	/* Internal */
#include "_map.h"

	/* External */
		//...

/* ----| Prototypes |----- */
	//...

/* ----| Internals  |----- */
	//...

/* ----| Public     |----- */

int	_hash_map_add(
	t_hash_map *const	map,
	char *const			key,
	void *const			value
)
{
	t_hash_map_entry	*new;

	if (_entry_find(map, key))
		return_error(EEXIST, key, EEXIST);
	if (map->length == UINT_MAX)
		return_error(EOVERFLOW, key, EOVERFLOW);
	new = _hash_map_entry_new(key);
	if (unlikely(!new))
		return (setting->error.errnum);

	new->value = value;
	_hash_map_add_entry(map, new);

	return (error_none);
}

void	*_hash_map_find(
	t_hash_map *const	map,
	char *const			key
)
{
	t_hash_map_entry *const	this = _entry_find(map, key);

	if (likely(this))
		return (this->value);

	return (NULL);
}

/**
 * @brief	return a pointer to the value at the key index
*/
void	**_hash_map_find_ptr(
	t_hash_map *const	map,
	char *const			key
)
{
	t_hash_map_entry *const	this = _entry_find(map, key);

	if (likely(this))
		return (&this->value);

	return (NULL);
}

void	_hash_map_remove(
	t_hash_map *const	map,
	char *const			key
)
{
	if (!_entry_find(map, key))
		return ;
	_hash_map_free(map, key, false);
	map->length--;
}

bool	_hash_map_contains(
	const t_hash_map *const	map,
	const char *const		key
)
{
	return (_entry_find(map, key) != NULL);
}

int	_hash_map_replace(
	t_hash_map *const	map,
	const char *const	key,
	void *const			value
)
{
	t_hash_map_entry *const	this = _entry_find(map, key);

	if (unlikely(!this))
		return (-1);

	this->value = value;
	return (error_none);
}

/**
 * this function will have multiple known bucket size that reduce colition,
 * if `size` == -1, it will use the new bigger bucket size has the new size
*/
int	_hash_map_resize(
	t_hash_map *const	map,
	const ssize_t		size
)
{
	t_hash_map_entry	**back = map->map;
	const size_t		old_capacity = map->capacity;
	t_hash_map_entry	**new_buckets;
	size_t			new_capacity;
	size_t			i;

	if (size == -1)
	{
		if (old_capacity > (SIZE_MAX - 1) / 2)
			return_error(EOVERFLOW, NULL, EOVERFLOW);
		new_capacity = old_capacity * 2 + 1;
	}
	else
		new_capacity = (size_t)size;
	if (new_capacity > SIZE_MAX / sizeof(*new_buckets))
		return_error(EOVERFLOW, NULL, EOVERFLOW);
	if (new_capacity == old_capacity)
		return (0);
	new_buckets = setting->alloc(sizeof(*new_buckets) * new_capacity);
	if (!new_buckets)
		return_error(errno ? errno : ENOMEM, NULL,
			setting->error.errnum);
	memset(new_buckets, 0, sizeof(*new_buckets) * new_capacity);
	i = 0;
	while (i < old_capacity)
	{
		t_hash_map_entry	*this = back[i];

		while (this)
		{
			t_hash_map_entry *const	next = this->next;
			size_t pos;

			this->hash = setting->hash_string(this->key);
			pos = this->hash % new_capacity;
			this->next = new_buckets[pos];
			new_buckets[pos] = this;
			this = next;
		}
		i++;
	}
	map->map = new_buckets;
	map->capacity = new_capacity;
	setting->free(back);

	return (error_none);
}
