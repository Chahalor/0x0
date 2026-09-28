/**
 * @file types.h
 * @brief Public settings type for Genesis core.
 * @details Holds the default dependencies consumed by core modules.
 */
#ifndef GENESIS_CORE_SETTING_TYPES_H
# define GENESIS_CORE_SETTING_TYPES_H
# pragma once

/* ************************************************************************** */
/*                                 Headers                                    */
/* ************************************************************************** */

/* -----| Systems   |----- */
# include <stddef.h>

/* -----| Modules   |----- */
# include "../allocator/types.h"

/* ************************************************************************** */
/*                                 Typedefs                                   */
/* ************************************************************************** */

/* -----| Structs   |----- */
typedef struct s_genesis_core_settings	t_genesis_core_settings;
typedef struct s_genesis_setting_key	t_genesis_setting_key;
typedef struct s_genesis_settings	t_genesis_settings;

/* ************************************************************************** */
/*                                 Enums                                      */
/* ************************************************************************** */

enum e_genesis_core_setting_error
{
	SETTING_ERROR_SUCCESS			= 0,
	SETTING_ERROR_NOT_CONFIGURED	= 1,
	SETTING_ERROR_UNKNOWN			= 2
};
typedef enum e_genesis_core_setting_error	t_genesis_core_setting_error;

/* ************************************************************************** */
/*                                 Structs                                    */
/* ************************************************************************** */

/** @brief Compatibility settings value for direct allocator configuration. */
struct s_genesis_core_settings
{
	t_genesis_core_allocator	allocator;
};

/**
 * @brief Stable descriptor exported by a module's setting.h.
 * @details A key is identified by its address and must outlive every manager
 * using it. Defaults and validation follow the signed error convention.
 */
struct s_genesis_setting_key
{
	const char	*name;
	size_t		size;
	int			(*defaults)(
		void	*output
	);
	int			(*validate)(
		const void	*value
	);
};

/**
 * @brief Collection of module settings owned by the caller.
 * @details Initialize before use, deinitialize before disposal, and do not
 * copy a live manager. Each module value is copied into private storage.
 * Shared access requires external synchronization.
 */
struct s_genesis_settings
{
	void	*entries;
};

#endif
