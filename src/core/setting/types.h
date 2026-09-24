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

/* -----| Modules   |----- */
# include "../allocator/types.h"

/* ************************************************************************** */
/*                                 Typedefs                                   */
/* ************************************************************************** */

/* -----| Enums     |----- */
typedef enum e_genesis_core_setting_error	t_genesis_core_setting_error;

/* -----| Structs   |----- */
typedef struct s_genesis_core_settings	t_genesis_core_settings;

/* ************************************************************************** */
/*                                 Enums                                      */
/* ************************************************************************** */

enum e_genesis_core_setting_error
{
	SETTING_ERROR_SUCCESS = 0,
	SETTING_ERROR_UNKNOWN = 1
};

/* ************************************************************************** */
/*                                 Structs                                    */
/* ************************************************************************** */

/** @brief Settings copied by value when a module captures them. */
struct s_genesis_core_settings
{
	t_genesis_core_allocator	allocator;
};

#endif
