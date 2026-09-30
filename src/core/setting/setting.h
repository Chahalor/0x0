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
// # include "types.h"

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
	struct
	{
		void	*(*alloc)(size_t);
		void	(*free)(void *);
		void	*(*realloc)(void *, size_t);
	}	allocator;
	// int							(*apply)(void);
};

/**
 * @brief Public workspace for Genesis-wide defaults.
 * 
 * @details Existing module allocations retain their captured allocator when
 * this value changes.
 * 
 * @par Example
 * 
 * @code{.c}
 * genesis_global_settings.allocator = allocator;
 * genesis_global_settings.apply();
 * @endcode
 */
extern t_genesis_global_settings	genesis_global_settings;
# define	G_SETTINGS				genesis_global_settings

#endif
