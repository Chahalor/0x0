/**
 * @file setting.h
 * @brief Global Genesis defaults and legacy settings compatibility API.
 * @details This module owns values shared across Genesis. Module behavior
 * settings belong to each module. The caller-owned manager below remains
 * only for existing core consumers during migration.
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
/*                                 Settings                                   */
/* ************************************************************************** */

/**
 * @brief Mutable public workspace for values shared across Genesis.
 * @details A zero allocator selects the system allocator. Direct
 * edits have no effect until apply() succeeds. Synchronize edits and apply
 * with concurrent module operations.
 */
typedef struct s_genesis_global_settings	t_genesis_global_settings;

struct s_genesis_global_settings
{
	t_genesis_core_allocator	allocator;
	int							(*apply)(void);
};

/**
 * @brief Public workspace for Genesis-wide defaults.
 * @details Existing module allocations retain their captured allocator when
 * this value changes.
 * @par Example
 * @code{.c}
 * genesis_global_settings.allocator = allocator;
 * genesis_global_settings.apply();
 * @endcode
 */
extern t_genesis_global_settings	genesis_global_settings;
# define	G_SETTINGS				genesis_global_settings

/**
 * @brief Validate and publish the public global settings without arguments.
 * @return 0 or EINVAL for an invalid allocator. Failure preserves the
 * previously applied global values. A zero descriptor restores the system
 * allocator. Future internal allocations that read this global value use
 * the new descriptor; existing allocations keep their original owner.
 * @par Example
 * @code{.c}
 * genesis_global_settings.allocator = allocator;
 * genesis_global_settings_apply();
 * @endcode
 */
int	genesis_global_settings_apply(void);

/**
 * @brief Copy the last applied global default allocator.
 * @return Valid allocator descriptor; the system allocator before the first
 * apply and after applying a zero descriptor.
 * @par Example
 * @code{.c}
 * t_genesis_core_allocator allocator = genesis_default_allocator();
 * @endcode
 */
t_genesis_core_allocator	genesis_default_allocator(void);

/**
 * @brief Create a compatibility settings value using the applied global default.
 * @return Settings value with a captured allocator descriptor.
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

/**
 * @brief Initialize a caller-owned settings manager.
 * @param settings Uninitialized manager storage. Do not initialize twice.
 * @return 0 or positive EINVAL.
 * @par Example
 * @code{.c}
 * t_genesis_settings settings;
 * genesis_settings_init(&settings);
 * @endcode
 */
int	genesis_settings_init(
	t_genesis_settings	*const	settings
);

/**
 * @brief Release every module override stored by the manager.
 * @param settings Initialized manager; NULL is accepted.
 * @return Nothing. A module's default remains available through its key.
 * @par Example
 * @code{.c}
 * genesis_settings_deinit(&settings);
 * @endcode
 */
void	genesis_settings_deinit(
	t_genesis_settings	*const	settings
);

/**
 * @brief Validate and copy a module's complete settings value.
 * @param settings Initialized manager to update.
 * @param key Stable key exported by the module's setting.h.
 * @param value Pointer to the module's settings value.
 * @param value_size Size of the object pointed to by value, in bytes.
 * @return 0, positive EINVAL for invalid arguments or a size mismatch,
 * ENOMEM/EOVERFLOW, or a module validation code. On error, the previous
 * setting remains unchanged.
 * @par Example
 * @code{.c}
 * genesis_settings_set(&settings, &genesis_allocator_setting,
 *     &allocator, sizeof(allocator));
 * @endcode
 */
int	genesis_settings_set(
	t_genesis_settings			*const	settings,
	const t_genesis_setting_key	*const	key,
	const void					*const	value,
	const size_t						value_size
);

/**
 * @brief Copy one module's override or its default into caller storage.
 * @param settings Initialized manager to read.
 * @param key Stable key exported by the module's setting.h.
 * @param output Writable storage for the module's settings value.
 * @param output_size Size of the object pointed to by output, in bytes.
 * @return 0, positive EINVAL for invalid arguments or a size mismatch,
 * a module default callback result, or
 * -SETTING_ERROR_NOT_CONFIGURED when no value or default exists.
 * @par Example
 * @code{.c}
 * genesis_settings_get(&settings, &genesis_allocator_setting,
 *     &allocator, sizeof(allocator));
 * @endcode
 */
int	genesis_settings_get(
	const t_genesis_settings		*const	settings,
	const t_genesis_setting_key		*const	key,
	void							*const	output,
	const size_t							output_size
);

/**
 * @brief Remove a module override and restore use of its default.
 * @param settings Initialized manager to update.
 * @param key Stable module key to reset.
 * @return 0 or positive EINVAL. Resetting an absent key succeeds.
 * @par Example
 * @code{.c}
 * genesis_settings_reset(&settings, &genesis_allocator_setting);
 * @endcode
 */
int	genesis_settings_reset(
	t_genesis_settings			*const	settings,
	const t_genesis_setting_key	*const	key
);

#endif
