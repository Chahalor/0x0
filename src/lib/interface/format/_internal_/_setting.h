/**
 * @file _setting.h
 * @brief Private applied configuration for the format module.
 * @details The public settings workspace is never read by runtime formatting.
 */
#ifndef GENESIS_INTERFACE_FORMAT_INTERNAL_SETTING_H
# define GENESIS_INTERFACE_FORMAT_INTERNAL_SETTING_H
# pragma once

/* -----| Systems   |----- */
# include <stddef.h>

/* -----| Typedefs  |----- */
typedef struct s_format_config	t_format_config;

/* -----| Structs   |----- */
struct s_format_config
{
	size_t						max_output;
};

/* -----| Prototypes |----- */
int	_format_settings_apply(void);
int	_format_settings_snapshot(
	t_format_config	*output
);

#endif
