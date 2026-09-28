/**
 * @file _setting.h
 * @brief Private default and validation of buffer growth settings.
 * @details Used only by the buffer module's public setting key.
 */
#ifndef GENESIS_CORE_BUFFER_INTERNAL_SETTING_H
# define GENESIS_CORE_BUFFER_INTERNAL_SETTING_H
# pragma once

/* ************************************************************************** */
/*                                 Headers                                    */
/* ************************************************************************** */

/* -----| Internals |----- */
# include "../types.h"

/* ************************************************************************** */
/*                                 Prototypes                                 */
/* ************************************************************************** */

t_genesis_core_buffer_settings	_buffer_settings_default(void);
int	_buffer_setting_defaults(
	void	*const	output
);
int	_buffer_setting_validate(
	const void	*const	value
);

#endif
