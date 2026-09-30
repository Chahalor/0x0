/**
 * @file _format.h
 * @brief Private engine and dynamic-result release operations.
 * @details Private only; depends on the central formatter state.
 */
#ifndef GENESIS_INTERFACE_FORMAT_INTERNAL_H
# define GENESIS_INTERFACE_FORMAT_INTERNAL_H

/* -----| Core      |----- */
# include "../../../../core/allocator/types.h"

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

// MARK: Errors

const char	*_format_strerror(
	int	code
);

// MARK: Registry

t_format_callback	format_registry_find(
	const char	*tag,
	size_t		length
);
int	format_registry_set(
	const char						*tag,
	t_format_callback				callback,
	const t_genesis_core_allocator	*allocator
);
int	format_registry_remove(
	const char	*tag
);

#endif
