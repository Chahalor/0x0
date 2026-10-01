/**
 * @file setting.h
 * @brief Global allocator callbacks for Genesis.
 * @details Module behavior settings belong to their modules.
 */
#ifndef GENESIS_CORE_SETTING_H
# define GENESIS_CORE_SETTING_H
# pragma once

/* ************************************************************************** */
/*                                 Headers                                    */
/* ************************************************************************** */

/* -----| Internals |----- */
# include <stddef.h>

/* ************************************************************************** */
/*                                 Macros                                     */
/* ************************************************************************** */

/* ************************************************************************** */
/*                                 Settings                                   */
/* ************************************************************************** */

/**
 * @brief Mutable public workspace for values shared across Genesis.
 * @details The callbacks initially use malloc, free and realloc. Replace
 * all callbacks together before allocating through Genesis.
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
};

/**
 * @brief Public workspace for Genesis-wide defaults.
 * 
 * @details Modules read these callbacks directly. Synchronize changes with
 * operations in other threads; existing allocations need their original free.
 * 
 * @par Example
 * 
 * @code{.c}
 * G_SETTINGS.allocator.alloc = custom_alloc;
 * G_SETTINGS.allocator.free = custom_free;
 * G_SETTINGS.allocator.realloc = custom_realloc;
 * @endcode
 */
extern t_genesis_global_settings	genesis_global_settings;
# define	G_SETTINGS				genesis_global_settings

#endif
