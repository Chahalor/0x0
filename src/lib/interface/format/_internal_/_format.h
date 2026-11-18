/**
 * @file _format.h
 * @brief Private engine and dynamic-result release operations.
 * @details Private only; depends on the central formatter state.
 */
#ifndef GENESIS_INTERFACE_FORMAT_INTERNAL_H
# define GENESIS_INTERFACE_FORMAT_INTERNAL_H

/* -----| Core      |----- */
# include "../../../../core/setting/setting.h"

/* -----| Internals |----- */
# include "_types.h"

int	genesis_format_(
	t_genesis_format_	*state
);
// MARK: API

char	*_format_vformat(
	const char			*format,
	va_list				args
);
size_t	_format_vsformat(
	char				*output,
	size_t				size,
	const char			*format,
	va_list				args
);
int	_format_set(
	const char			*tag,
	t_format_callback		callback
);
/** @brief Register a private tag that consumes no value argument. */
int	_format_set_safe(
	const char				*tag,
	t_format_safe_callback	callback
);

// MARK: Errors

const char	*_format_strerror(
	int	code
);

// MARK: Registry

const t_format_tag_entry	*format_registry_find(
	const char	*tag,
	size_t		length
);
int	format_registry_set(
	const char					*tag,
	t_format_callback			callback,
	t_format_safe_callback		safe_callback,
	const t_format_allocator	*allocator
);
int	format_registry_remove(
	const char	*tag
);

// MARK: Buitlins

char	*genesis_format_builtin_colors(
	const char	*param
);

char	*genesis_format_builtin_uuid(
	const void	*value,
	const char	*param
);

char	*genesis_format_builtin_json(
	const void	*value,
	const char	*param
);

char	*genesis_format_builtin_toml(
	const void	*value,
	const char	*param
);

void	genesis_format_builtins_init(void);

#endif
