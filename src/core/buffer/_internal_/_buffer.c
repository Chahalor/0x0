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

	/* External */
# include "../error.h"
# include "../setting.h"
# include "../../setting/setting.h"

/* ----| Prototypes |----- */
static int	_buffer_source_offset(
	const t_genesis_core_buffer	*const	buffer,
	const void					*const	data,
	const size_t						length,
	size_t						*const	offset
);

/* ----| Internals  |----- */
/** @brief Identify an internal source and reject bytes outside logical length. */
static int	_buffer_source_offset(
	const t_genesis_core_buffer	*const	buffer,
	const void					*const	data,
	const size_t						length,
	size_t						*const	offset
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
	);
}

/** @brief Initialize storage using the default core settings. */
int	_buffer_init(
	t_genesis_core_buffer	*const	buffer,
	const size_t					capacity
)
{
	unsigned char	*bytes;

	if (capacity == 0)
	{
		*buffer = (t_genesis_core_buffer){0};
		return (0);
	}
	if (!G_SETTINGS.allocator.alloc || !G_SETTINGS.allocator.free)
		return (EINVAL);

	bytes = G_SETTINGS.allocator.alloc(capacity);
	if (!bytes)
		return (ENOMEM);

	buffer->bytes = bytes;
	buffer->capacity = capacity;
	buffer->length = 0;

	return (0);
}

/** @brief Release byte storage and reset a caller-owned buffer. */
void	_buffer_deinit(
	t_genesis_core_buffer	*const	buffer
)
{
	if (buffer->bytes)
		G_SETTINGS.allocator.free(buffer->bytes);
	*buffer = (t_genesis_core_buffer){0};
}

/** @brief Allocate a buffer object using the current global callbacks. */
t_genesis_core_buffer	*_buffer_new(
	const size_t	capacity
)
{
	t_genesis_core_buffer	*result;

	if (!G_SETTINGS.allocator.alloc || !G_SETTINGS.allocator.free)
	{
		errno = EINVAL;
		return (NULL);
	}
	result = G_SETTINGS.allocator.alloc(sizeof(t_genesis_core_buffer));
	if (!result)
	{
		errno = ENOMEM;
		return (NULL);
	}

	*result = (t_genesis_core_buffer){0};
	if (capacity != 0 && _buffer_init(result, capacity) != 0)
	{
		G_SETTINGS.allocator.free(result);
		errno = ENOMEM;
		return (NULL);
	}

	return (result);
}

/** @brief Release object and bytes using the current global callback. */
void	_buffer_free(
	t_genesis_core_buffer	*const	buffer
)
{
	if (!buffer)
		return ;

	if (buffer->bytes)
		G_SETTINGS.allocator.free(buffer->bytes);

	G_SETTINGS.allocator.free(buffer);
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

	if (genesis_buffer_setting.growth == BUFFER_GROWTH_EXACT)
		capacity = min_capacity;
	else
	{
		capacity = buffer->capacity
			? buffer->capacity
			: genesis_buffer_setting.initial_capacity;
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

	if (!G_SETTINGS.allocator.realloc)
		return (EINVAL);
	bytes = G_SETTINGS.allocator.realloc(buffer->bytes, capacity);
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

	old_length = buffer->length;
	if (new_length > old_length)
	{
		int	result;

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

	source_is_internal = _buffer_source_offset(buffer, data, length, &source_offset);
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
