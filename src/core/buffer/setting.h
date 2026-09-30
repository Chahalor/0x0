/**
 * @file setting.h
 * @brief Public growth policy key for Genesis byte buffers.
 * @details Configure buffer growth through the shared settings manager.
 */
#ifndef GENESIS_CORE_BUFFER_SETTING_H
# define GENESIS_CORE_BUFFER_SETTING_H
# pragma once

/* ************************************************************************** */
/*                                 Headers                                    */
/* ************************************************************************** */

/* -----| Internals |----- */
# include "types.h"

/* ************************************************************************** */
/*                                 Prototypes                                 */
/* ************************************************************************** */

/**
 * @brief Key for a buffer's capacity growth policy.
 * @details The value type is t_genesis_core_buffer_settings. The default
 * starts at 64 bytes and doubles capacity. Exact growth uses requested size.
 * @par Example
 * @code{.c}
 * t_genesis_core_buffer_settings value = {64, BUFFER_GROWTH_EXACT};
 * genesis_settings_set(&settings, &genesis_buffer_setting,
 *     &value, sizeof(value));
 * @endcode
 */
extern const t_genesis_core_buffer_settings	genesis_buffer_setting;

#endif
