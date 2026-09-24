/**
 * @file types.h
 * @brief Public identifiers for Genesis core error dispatch.
 * @details Defines the modules accepted by the central error dispatcher.
 */
#ifndef GENESIS_CORE_ERROR_TYPES_H
# define GENESIS_CORE_ERROR_TYPES_H
# pragma once

/* ************************************************************************** */
/*                                 Headers                                    */
/* ************************************************************************** */

/* ************************************************************************** */
/*                                 Typedefs                                   */
/* ************************************************************************** */

/* -----| Enums     |----- */
typedef enum e_genesis_core_module	t_genesis_core_module;
typedef enum e_genesis_core_error	t_genesis_core_error;

/* ************************************************************************** */
/*                                 Enums                                      */
/* ************************************************************************** */

enum e_genesis_core_module
{
	GENESIS_MODULE_ERROR = 0,
	GENESIS_MODULE_SETTING,
	GENESIS_MODULE_ALLOCATOR,
	GENESIS_MODULE_BUFFER
};

enum e_genesis_core_error
{
	CORE_ERROR_SUCCESS = 0,
	CORE_ERROR_UNKNOWN = 1
};

#endif
