/**
 * @file setting.h
 * @brief Public growth policy for Genesis byte buffers.
 * @details Exposes the fixed default growth policy used by new buffers.
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
 * @brief Default capacity growth policy.
 * @details Starts at 64 bytes and doubles capacity. This value is constant.
 * @par Example
 * @code{.c}
 * size_t initial = genesis_buffer_setting.initial_capacity;
 * @endcode
 */
extern const t_genesis_core_buffer_settings	genesis_buffer_setting;

#endif
