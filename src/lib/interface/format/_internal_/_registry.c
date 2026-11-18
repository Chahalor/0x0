/**
 * @file _registry.c
 * @brief Hashed custom tags with allocator ownership per allocation.
 */

/* ----| Headers    |----- */
	/* Standard */
# include <errno.h>
# include <stdint.h>
# include <string.h>

	/* Internal */
# include "_format.h"

	/* External */


/* ----| State      |----- */
static t_format_registry	registry;

/* ----| Prototypes |----- */
static uint64_t	format_tag_hash(
	const char	*tag,
	size_t		length
);
static int	format_tag_length(
	const char	*tag,
	size_t		*length
);
static int	format_registry_grow(
	const t_format_allocator	*allocator
);
static void	format_tag_free(
	t_format_tag_entry	*entry
);

/* ----| Internals  |----- */
/** @brief FNV-1a hash over exactly length tag bytes. */
static uint64_t	format_tag_hash(
	const char	*tag,
	size_t		length
)
{
	uint64_t	hash = UINT64_C(14695981039346656037);
	size_t		index;

	for (index = 0; index < length; ++index)
	{
		hash ^= (unsigned char)tag[index];
		hash *= UINT64_C(1099511628211);
	}

	return (hash);
}

/** @brief Validate tag characters and return its byte length. */
static int	format_tag_length(
	const char	*tag,
	size_t		*length
)
{
	size_t	index = 0;

	while (tag[index])
	{
		if (tag[index] == ':' || tag[index] == '{' || tag[index] == '}')
			return (EINVAL);
		if (index == SIZE_MAX - 1)
			return (EOVERFLOW);
		++index;
	}

	if (index == 0)
		return (EINVAL);
	*length = index;
	return (0);
}

/** @brief Grow buckets without changing tag entries or their allocators. */
static int	format_registry_grow(
	const t_format_allocator	*allocator
)
{
	t_format_tag_entry	**replacement;
	t_format_tag_entry	*entry;
	t_format_tag_entry	*following;
	size_t				new_count;
	size_t				index;

	if (registry.bucket_count > SIZE_MAX / 2)
		return (EOVERFLOW);

	new_count = registry.bucket_count ? registry.bucket_count * 2 : 16;
	if (new_count > SIZE_MAX / sizeof(*replacement))
		return (EOVERFLOW);
	replacement = allocator->alloc(new_count * sizeof(*replacement));
	if (!replacement)
		return (ENOMEM);
	memset(replacement, 0, new_count * sizeof(*replacement));

	for (index = 0; index < registry.bucket_count; ++index)
	{
		entry = registry.buckets[index];
		while (entry)
		{
			following = entry->next;
			entry->next = replacement[entry->hash % new_count];
			replacement[entry->hash % new_count] = entry;
			entry = following;
		}
	}

	if (registry.buckets)
		registry.bucket_allocator.free(registry.buckets);
	registry.buckets = replacement;
	registry.bucket_count = new_count;
	registry.bucket_allocator = *allocator;
	return (0);
}

/** @brief Release an entry with the allocator captured on insertion. */
static void	format_tag_free(
	t_format_tag_entry	*entry
)
{
	entry->allocator.free(entry->tag);
	entry->allocator.free(entry);
}

/* ----| Public     |----- */
/** @brief Look up a borrowed tag entry in expected constant time. */
const t_format_tag_entry	*format_registry_find(
	const char	*tag,
	size_t		length
)
{
	t_format_tag_entry	*entry;
	uint64_t			hash;

	if (registry.bucket_count == 0)
		return (NULL);

	hash = format_tag_hash(tag, length);
	entry = registry.buckets[hash % registry.bucket_count];
	while (entry)
	{
		if (entry->hash == hash && entry->length == length
			&& memcmp(entry->tag, tag, length) == 0)
			return (entry);
		entry = entry->next;
	}

	return (NULL);
}

/** @brief Add or replace a tag, preserving all prior entries on failure. */
int	format_registry_set(
	const char					*tag,
	t_format_callback			callback,
	t_format_safe_callback		safe_callback,
	const t_format_allocator	*allocator
)
{
	t_format_tag_entry	*entry;
	uint64_t			hash;
	size_t				length;
	int					status;

	if (!tag || !allocator || !allocator->alloc || !allocator->free
		|| (callback == NULL) == (safe_callback == NULL))
		return (EINVAL);

	status = format_tag_length(tag, &length);
	if (status != 0)
		return (status);

	hash = format_tag_hash(tag, length);
	if (registry.bucket_count != 0)
	{
		entry = registry.buckets[hash % registry.bucket_count];
		while (entry)
		{
			if (entry->hash == hash && entry->length == length
				&& memcmp(entry->tag, tag, length) == 0)
			{
				entry->callback = callback;
				entry->safe_callback = safe_callback;
				return (0);
			}
			entry = entry->next;
		}
	}

	entry = allocator->alloc(sizeof(*entry));
	if (!entry)
		return (ENOMEM);

	entry->allocator = *allocator;
	entry->length = length;
	entry->tag = allocator->alloc(length + 1);
	if (!entry->tag)
	{
		allocator->free(entry);
		return (ENOMEM);
	}
	memcpy(entry->tag, tag, length + 1);
	if (registry.bucket_count == 0
		|| registry.tag_count >= registry.bucket_count
			- registry.bucket_count / 4)
	{
		status = format_registry_grow(allocator);
		if (status != 0)
		{
			format_tag_free(entry);
			return (status);
		}
	}

	entry->hash = hash;
	entry->callback = callback;
	entry->safe_callback = safe_callback;
	entry->next = registry.buckets[hash % registry.bucket_count];
	registry.buckets[hash % registry.bucket_count] = entry;
	++registry.tag_count;
	return (0);
}

/** @brief Remove a tag and release its captured allocation. */
int	format_registry_remove(
	const char	*tag
)
{
	t_format_tag_entry	**link;
	t_format_tag_entry	*entry;
	uint64_t			hash;
	size_t				length;
	int					status;

	status = format_tag_length(tag, &length);
	if (status != 0)
		return (status);

	if (registry.bucket_count == 0)
		return (-FORMAT_ERROR_UNKNOWN_TAG);

	hash = format_tag_hash(tag, length);
	link = &registry.buckets[hash % registry.bucket_count];
	while (*link)
	{
		entry = *link;
		if (entry->hash == hash && entry->length == length && memcmp(entry->tag, tag, length) == 0)
		{
			*link = entry->next;
			format_tag_free(entry);
			--registry.tag_count;
			if (registry.tag_count == 0)
			{
				registry.bucket_allocator.free(registry.buckets);
				registry.buckets = NULL;
				registry.bucket_count = 0;
			}

			return (0);
		}
		link = &entry->next;
	}

	return (-FORMAT_ERROR_UNKNOWN_TAG);
}
