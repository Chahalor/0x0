/**
 * @file setting.h
 * @brief Public configuration of Genesis core defaults.
 * @details Owns the default allocator descriptor used by other core modules.
 */
#ifndef GENESIS_CORE_SETTING_H
# define GENESIS_CORE_SETTING_H
# pragma once

/* ************************************************************************** */
/*                                 Headers                                    */
/* ************************************************************************** */

/* -----| Internals |----- */
# include "types.h"

/* ************************************************************************** */
/*                                 Macros                                     */
/* ************************************************************************** */

/* ************************************************************************** */
/*                                 Prototypes                                 */
/* ************************************************************************** */

/**
 * @brief Create independent core settings using the system allocator.
 * @return Settings value with a malloc-backed allocator.
 * @par Example
 * @code{.c}
 * t_genesis_core_settings settings = core_settings_default();
 * @endcode
 */
t_genesis_core_settings	core_settings_default(void);

/**
 * @brief Replace the allocator in caller-owned settings.
 * @param settings Settings object to update.
 * @param allocator Valid descriptor to copy into settings.
 * @return 0 on success or positive EINVAL for invalid arguments.
 * @par Example
 * @code{.c}
 * core_settings_set_allocator(&settings, &custom_allocator);
 * @endcode
 */
int	core_settings_set_allocator(
	t_genesis_core_settings			*const	settings,
	const t_genesis_core_allocator	*const	allocator
);

/**
 * @brief Borrow the allocator stored in settings.
 * @param settings Settings object to inspect.
 * @return Pointer to its descriptor, or NULL with errno set to EINVAL.
 * @par Example
 * @code{.c}
 * const t_genesis_core_allocator *a = core_settings_allocator(&settings);
 * @endcode
 */
const t_genesis_core_allocator	*core_settings_allocator(
	const t_genesis_core_settings	*const	settings
);

#endif
