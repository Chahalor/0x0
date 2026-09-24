/**
 * @file types.h
 * @brief Public types for extensible Genesis error dispatch.
 * @details Each module exports a stable descriptor with its message function.
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

typedef struct s_genesis_error_module	t_genesis_error_module;

/* ************************************************************************** */
/*                                 Enums                                      */
/* ************************************************************************** */

enum e_genesis_core_error
{
	CORE_ERROR_SUCCESS	= 0,
	CORE_ERROR_UNKNOWN	= 1
};
typedef enum e_genesis_core_error	t_genesis_core_error;

/* ************************************************************************** */
/*                                 Structs                                    */
/* ************************************************************************** */

/**
 * @brief Stable module descriptor for the central error interface.
 * @details name and describe must be non-NULL. The descriptor must outlive
 * calls that use it. Modules normally export one constant descriptor.
 */
struct s_genesis_error_module
{
	const char	*name;
	const char	*(*describe)(
		int	code
	);
};

#endif
