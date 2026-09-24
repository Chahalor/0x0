/**
 * @file setting.h
 * @brief Public allocator configuration key for Genesis settings.
 * @details The value type is t_genesis_core_allocator from this module.
 */
#ifndef GENESIS_CORE_ALLOCATOR_SETTING_H
# define GENESIS_CORE_ALLOCATOR_SETTING_H
# pragma once

/* ************************************************************************** */
/*                                 Headers                                    */
/* ************************************************************************** */

/* -----| Internals |----- */
# include "types.h"

/* -----| Modules   |----- */
# include "../setting/types.h"

/* ************************************************************************** */
/*                                 Prototypes                                 */
/* ************************************************************************** */

/**
 * @brief Key for configuring the allocator used by modules.
 * @details Use genesis_settings_set/get with a t_genesis_core_allocator value.
 * An unset key produces the system allocator.
 * @par Example
 * @code{.c}
 * genesis_settings_set(&settings, &genesis_allocator_setting,
 *     &allocator, sizeof(allocator));
 * @endcode
 */
extern const t_genesis_setting_key	genesis_allocator_setting;

#endif
