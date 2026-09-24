/**
 * @file _buffer.c
 * @brief Private byte buffer storage and growth logic.
 */

/* ----| Headers    |----- */
	/* Standard */
# include <errno.h>
# include <stdint.h>
# include <string.h>

	/* Internal */
# include "_buffer.h"
# include "_setting.h"

	/* External */
# include "../../allocator/allocator.h"
# include "../../allocator/setting.h"
# include "../error.h"
# include "../setting.h"
# include "../../setting/setting.h"

/* ----| Prototypes |----- */
static int	_buffer_source_offset(
	const t_genesis_core_buffer	*const	buffer,
	const void					*const	data,
	const size_t						length,
	size_t							*const	offset
);

/* ----| Internals  |----- */
/** @brief Identify an internal source and reject bytes outside logical length. */
static int	_buffer_source_offset(
	const t_genesis_core_buffer	*const	buffer,
	const void					*const	data,
	const size_t						length,
	size_t							*const	offset
)
{
	uintptr_t	base;
	uintptr_t	address;

	if (!buffer->bytes || length == 0)
		return (0);

	base = (uintptr_t)buffer->bytes;
	address = (uintptr_t)data;
	if (address < base || address - base >= buffer->capacity)
		return (0);

	*offset = (size_t)(address - base);
	if (*offset > buffer->length || length > buffer->length - *offset)
		return (-BUFFER_ERROR_SOURCE_OUT_OF_RANGE);

	return (1);
}

/** @brief Validate invariants before an existing buffer is used. */
int	_buffer_is_valid(
	const t_genesis_core_buffer	*const	buffer
)
{
	return (buffer && buffer->length <= buffer->capacity
		&& (buffer->capacity == 0 || buffer->bytes != NULL)
		&& _buffer_setting_validate(&buffer->settings) == 0
		&& core_allocator_validate(&buffer->allocator) == 0);
}

/** @brief Initialize storage using the default core settings. */
int	_buffer_init_default(
	t_genesis_core_buffer	*const	buffer,
	const size_t					capacity
)
{
	t_genesis_settings	settings;
	int					result;

	genesis_settings_init(&settings);
	result = _buffer_init_with_settings(buffer, capacity, &settings);
	genesis_settings_deinit(&settings);
	return (result);
}

/** @brief Capture an allocator and optionally allocate initial byte storage. */
int	_buffer_init_with_allocator(
	t_genesis_core_buffer			*const	buffer,
	const size_t							capacity,
	const t_genesis_core_allocator	*const	allocator
)
{
	*buffer = (t_genesis_core_buffer){
		.allocator = *allocator,
		.settings = _buffer_settings_default()
	};
	if (capacity == 0)
		return (0);

	buffer->bytes = core_alloc(&buffer->allocator, capacity);
	if (!buffer->bytes)
		return (ENOMEM);

	buffer->capacity = capacity;
	return (0);
}

/** @brief Resolve the allocator from a shared settings manager. */
int	_buffer_init_with_settings(
	t_genesis_core_buffer		*const	buffer,
	const size_t						capacity,
	const t_genesis_settings	*const	settings
)
{
	t_genesis_core_allocator	allocator;
	t_genesis_core_buffer_settings	buffer_settings;
	int						result;

	result = genesis_settings_get(settings, &genesis_allocator_setting,
		&allocator);
	if (result != 0)
		return (result);

	result = genesis_settings_get(settings, &genesis_buffer_setting,
		&buffer_settings);
	if (result != 0)
		return (result);

	result = _buffer_init_with_allocator(buffer, capacity, &allocator);
	if (result == 0)
		buffer->settings = buffer_settings;

	return (result);
}

/** @brief Release byte storage and reset a caller-owned buffer. */
void	_buffer_deinit(
	t_genesis_core_buffer	*const	buffer
)
{
	core_free(&buffer->allocator, buffer->bytes, buffer->capacity);
	*buffer = (t_genesis_core_buffer){0};
}

/** @brief Allocate a buffer object using default core settings. */
t_genesis_core_buffer	*_buffer_new_default(
	const size_t	capacity
)
{
	t_genesis_settings		settings;
	t_genesis_core_buffer	*buffer;

	genesis_settings_init(&settings);
	buffer = _buffer_new_with_settings(capacity, &settings);
	genesis_settings_deinit(&settings);
	return (buffer);
}

/** @brief Allocate the object and its bytes with one allocator. */
t_genesis_core_buffer	*_buffer_new_with_allocator(
	const size_t							capacity,
	const t_genesis_core_allocator	*const	allocator
)
{
	t_genesis_core_buffer	*buffer;
	int						result;

	buffer = core_alloc(allocator, sizeof(*buffer));
	if (!buffer)
		return (NULL);

	result = _buffer_init_with_allocator(buffer, capacity, allocator);
	if (result != 0)
	{
		core_free(allocator, buffer, sizeof(*buffer));
		errno = result;
		return (NULL);
	}

	return (buffer);
}

/** @brief Resolve an allocator and create an owned buffer. */
t_genesis_core_buffer	*_buffer_new_with_settings(
	const size_t						capacity,
	const t_genesis_settings	*const	settings
)
{
	t_genesis_core_allocator	allocator;
	t_genesis_core_buffer_settings	buffer_settings;
	t_genesis_core_buffer		*buffer;
	int						result;

	result = genesis_settings_get(settings, &genesis_allocator_setting,
		&allocator);
	if (result != 0)
	{
		errno = result > 0 ? result : EINVAL;
		return (NULL);
	}

	result = genesis_settings_get(settings, &genesis_buffer_setting,
		&buffer_settings);
	if (result != 0)
	{
		errno = result > 0 ? result : EINVAL;
		return (NULL);
	}

	buffer = _buffer_new_with_allocator(capacity, &allocator);
	if (buffer)
		buffer->settings = buffer_settings;

	return (buffer);
}

/** @brief Release object and bytes using the captured allocator. */
void	_buffer_free(
	t_genesis_core_buffer	*const	buffer
)
{
	t_genesis_core_allocator	allocator;

	allocator = buffer->allocator;
	_buffer_deinit(buffer);
	core_free(&allocator, buffer, sizeof(*buffer));
}

/** @brief Grow capacity geometrically without changing logical length. */
int	_buffer_reserve(
	t_genesis_core_buffer	*const	buffer,
	const size_t					min_capacity
)
{
	size_t	capacity;
	void	*bytes;

	if (min_capacity <= buffer->capacity)
		return (0);

	if (buffer->settings.growth == BUFFER_GROWTH_EXACT)
		capacity = min_capacity;
	else
	{
		capacity = buffer->capacity ? buffer->capacity
			: buffer->settings.initial_capacity;
		while (capacity < min_capacity)
		{
			if (capacity > SIZE_MAX / 2)
			{
				capacity = min_capacity;
				break;
			}

			capacity *= 2;
		}
	}

	bytes = core_realloc(&buffer->allocator, buffer->bytes,
		buffer->capacity, capacity);
	if (!bytes)
		return (ENOMEM);

	buffer->bytes = bytes;
	buffer->capacity = capacity;
	return (0);
}

/** @brief Change logical length and zero newly exposed bytes. */
int	_buffer_resize(
	t_genesis_core_buffer	*const	buffer,
	const size_t					new_length
)
{
	size_t	old_length;
	int		result;

	old_length = buffer->length;
	if (new_length > old_length)
	{
		result = _buffer_reserve(buffer, new_length);
		if (result != 0)
			return (result);

		memset(buffer->bytes + old_length, 0, new_length - old_length);
	}

	buffer->length = new_length;
	return (0);
}

/** @brief Append without growth, reporting insufficient capacity. */
int	_buffer_append_fixed(
	t_genesis_core_buffer	*const	buffer,
	const void				*const	data,
	const size_t					length
)
{
	size_t	source_offset;
	int		result;

	if (length > buffer->capacity - buffer->length)
		return (ENOSPC);

	result = _buffer_source_offset(buffer, data, length, &source_offset);
	if (result < 0)
		return (result);

	if (length != 0)
		memmove(buffer->bytes + buffer->length, data, length);

	buffer->length += length;
	return (0);
}

/** @brief Append bytes and preserve internal source pointers across growth. */
int	_buffer_append(
	t_genesis_core_buffer	*const	buffer,
	const void				*const	data,
	const size_t					length
)
{
	const unsigned char	*source			= data;
	size_t				source_offset	= 0;
	int					source_is_internal;
	size_t				new_length;
	int					result;

	if (length == 0)
		return (0);

	source_is_internal = _buffer_source_offset(buffer, data, length,
		&source_offset);
	if (source_is_internal < 0)
		return (source_is_internal);

	new_length = buffer->length + length;

	result = _buffer_reserve(buffer, new_length);
	if (result != 0)
		return (result);

	if (source_is_internal)
		source = buffer->bytes + source_offset;

	memmove(buffer->bytes + buffer->length, source, length);
	buffer->length = new_length;
	return (0);
}

/** @brief Reset length without erasing or releasing storage. */
void	_buffer_clear(
	t_genesis_core_buffer	*const	buffer
)
{
	buffer->length = 0;
}
