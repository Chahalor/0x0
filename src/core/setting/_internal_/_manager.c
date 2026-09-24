/**
 * @file _manager.c
 * @brief Private storage for independently configured modules.
 */

/* ----| Headers    |----- */
	/* Standard */
# include <errno.h>
# include <stdint.h>
# include <string.h>

	/* Internal */
# include "_manager.h"
# include "_types.h"

	/* External */
# include "../../allocator/allocator.h"
# include "../../allocator/setting.h"

/* ----| Prototypes |----- */
static t_genesis_setting_node	*_manager_find(
	const t_genesis_settings		*const	settings,
	const t_genesis_setting_key		*const	key
);

/* ----| Internals  |----- */
/** @brief Find one module override by descriptor identity. */
static t_genesis_setting_node	*_manager_find(
	const t_genesis_settings		*const	settings,
	const t_genesis_setting_key		*const	key
)
{
	t_genesis_setting_node	*entry;

	entry = settings->entries;
	while (entry)
	{
		if (entry->key == key)
			return (entry);

		entry = entry->next;
	}

	return (NULL);
}

/** @brief Initialize an empty manager without allocating memory. */
void	_manager_init(
	t_genesis_settings	*const	settings
)
{
	settings->entries = NULL;
}

/** @brief Release all stored module overrides. */
void	_manager_deinit(
	t_genesis_settings	*const	settings
)
{
	t_genesis_setting_node	*entry;
	t_genesis_setting_node	*next;

	entry = settings->entries;
	while (entry)
	{
		next = entry->next;
		core_free(&entry->allocator, entry,
			sizeof(*entry) + entry->key->size);
		entry = next;
	}

	settings->entries = NULL;
}

/** @brief Validate then copy a module value, preserving the old one on failure. */
int	_manager_set(
	t_genesis_settings			*const	settings,
	const t_genesis_setting_key	*const	key,
	const void					*const	value
)
{
	t_genesis_setting_node		*entry;
	t_genesis_core_allocator	storage_allocator;
	int							result;

	if (key->validate)
	{
		result = key->validate(value);
		if (result != 0)
			return (result);
	}

	entry = _manager_find(settings, key);
	if (!entry)
	{
		if (key->size > SIZE_MAX - sizeof(*entry))
			return (EOVERFLOW);

		result = _manager_get(settings, &genesis_allocator_setting,
			&storage_allocator);
		if (result != 0)
			return (result);

		entry = core_alloc(&storage_allocator, sizeof(*entry) + key->size);
		if (!entry)
			return (ENOMEM);

		entry->key = key;
		entry->next = settings->entries;
		entry->allocator = storage_allocator;
		settings->entries = entry;
	}

	memcpy(entry->value, value, key->size);
	return (0);
}

/** @brief Copy an override or ask the module for its default value. */
int	_manager_get(
	const t_genesis_settings		*const	settings,
	const t_genesis_setting_key		*const	key,
	void							*const	output
)
{
	const t_genesis_setting_node	*entry;

	entry = _manager_find(settings, key);
	if (entry)
	{
		memcpy(output, entry->value, key->size);
		return (0);
	}

	if (key->defaults)
		return (key->defaults(output));

	return (-SETTING_ERROR_NOT_CONFIGURED);
}

/** @brief Remove an override so the next read uses its module default. */
int	_manager_reset(
	t_genesis_settings			*const	settings,
	const t_genesis_setting_key	*const	key
)
{
	t_genesis_setting_node	*previous;
	t_genesis_setting_node	*entry;

	previous = NULL;
	entry = settings->entries;
	while (entry)
	{
		if (entry->key == key)
		{
			if (previous)
				previous->next = entry->next;
			else
				settings->entries = entry->next;

			core_free(&entry->allocator, entry,
				sizeof(*entry) + entry->key->size);
			return (0);
		}

		previous = entry;
		entry = entry->next;
	}

	return (0);
}
