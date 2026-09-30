/**
 * @file _setting.h
 * @brief Private settings operations.
 * @details These declarations belong only to the setting module.
 */
#ifndef GENESIS_CORE_INTERNAL_SETTING_H
# define GENESIS_CORE_INTERNAL_SETTING_H
# pragma once

/* ************************************************************************** */
/*                                 Headers                                    */
/* ************************************************************************** */

/* -----| Internals |----- */
# include "../types.h"

/* ************************************************************************** */
/*                                 Prototypes                                 */
/* ************************************************************************** */

t_genesis_core_settings		_settings_default(void);
int							_settings_global_apply(void);
t_genesis_core_allocator	_settings_global_allocator(void);
void						_settings_set_allocator(
	t_genesis_core_settings			*const	settings,
	const t_genesis_core_allocator	*const	allocator
);
const t_genesis_core_allocator	*_settings_allocator(
	const t_genesis_core_settings	*const	settings
);

#endif
