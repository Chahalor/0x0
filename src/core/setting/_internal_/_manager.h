/**
 * @file _manager.h
 * @brief Private settings manager operations.
 * @details Stores copies of module values behind public setting keys.
 */
#ifndef GENESIS_CORE_SETTING_INTERNAL_MANAGER_H
# define GENESIS_CORE_SETTING_INTERNAL_MANAGER_H
# pragma once

/* ************************************************************************** */
/*                                 Headers                                    */
/* ************************************************************************** */

/* -----| Internals |----- */
# include "../types.h"

/* ************************************************************************** */
/*                                 Prototypes                                 */
/* ************************************************************************** */

void	_manager_init(
	t_genesis_settings	*const	settings
);
void	_manager_deinit(
	t_genesis_settings	*const	settings
);
int		_manager_set(
	t_genesis_settings			*const	settings,
	const t_genesis_setting_key	*const	key,
	const void					*const	value
);
int		_manager_get(
	const t_genesis_settings		*const	settings,
	const t_genesis_setting_key	*const	key,
	void						*const	output
);
int		_manager_reset(
	t_genesis_settings			*const	settings,
	const t_genesis_setting_key	*const	key
);

#endif
