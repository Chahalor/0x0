/**
 * @file _setting.h
 * @brief Private allocator settings callbacks.
 * @details Used by the allocator module's public setting key.
 */
#ifndef GENESIS_CORE_ALLOCATOR_INTERNAL_SETTING_H
# define GENESIS_CORE_ALLOCATOR_INTERNAL_SETTING_H
# pragma once

/* ************************************************************************** */
/*                                 Prototypes                                 */
/* ************************************************************************** */

int	_allocator_setting_defaults(
	void	*const	output
);
int	_allocator_setting_validate(
	const void	*const	value
);

#endif
