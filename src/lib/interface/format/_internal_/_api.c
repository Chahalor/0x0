/**
 * @file _api.c
 * @brief Configure and execute in-memory formatting.
 */

/* ----| Headers    |----- */
	/* Standard */
# include <errno.h>
# include <stdint.h>

	/* Internal */
# include "_format.h"
# include "_setting.h"

	/* External */
# include "../../../../core/setting/setting.h"
# include "../../../memory.h"

/* ----| Prototypes |----- */
static int	format_configure(
	t_genesis_format_	*state
);

/* ----| Internals  |----- */
/** @brief Capture the format output limit for one operation. */
static int	format_configure(
	t_genesis_format_	*state
)
{
	t_format_config	value;
	int						status;

	status = _format_settings_snapshot(&value);
	if (status == 0)
	{
		state->max_output = value.max_output;
	}

	return (status);
}

/* ----| Public     |----- */
/** @brief Build an owned string with one copied argument list. */
char	*_format_vformat(
	const char			*format,
	va_list				args
)
{
	t_genesis_format_	state = {0};
	int					status;

	state.fm = format;
	status = format_configure(&state);
	if (status != 0)
	{
		errno = status > 0 ? status : EINVAL;
		return (NULL);
	}

	va_copy(state.list, args);
	status = genesis_format_(&state);
	va_end(state.list);
	if (status != 0)
	{
		mem_free(state.buffer);
		errno = status > 0 ? status : EINVAL;
		return (NULL);
	}

	return (state.buffer);
}

/** @brief Write to caller storage and preserve snprintf-style length. */
size_t	_format_vsformat(
	char				*output,
	size_t				size,
	const char			*format,
	va_list				args
)
{
	t_genesis_format_	state = {0};
	int					status;

	state.fm = format;
	state.buffer = output;
	state.capacity = size;
	state.fixed_capacity = true;
	if (size != 0)
		output[0] = '\0';

	status = format_configure(&state);
	if (status != 0)
	{
		errno = status > 0 ? status : EINVAL;
		return (SIZE_MAX);
	}

	va_copy(state.list, args);
	status = genesis_format_(&state);
	va_end(state.list);
	if (status != 0)
	{
		if (size != 0)
			output[0] = '\0';
		errno = status > 0 ? status : EINVAL;
		return (SIZE_MAX);
	}

	return (state.index);
}

/** @brief Register a tag using the applied global allocator. */
int	_format_set(
	const char			*tag,
	t_format_callback		callback
)
{
	t_format_allocator	allocator;
	allocator = (t_format_allocator){G_SETTINGS.allocator.alloc,
		G_SETTINGS.allocator.free};
	if (!allocator.alloc || !allocator.free)
		return (EINVAL);

	return (format_registry_set(tag, callback, &allocator));
}
