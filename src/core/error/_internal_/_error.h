/**
 * @file _error.h
 * @brief Private message dispatch for the core error module.
 * @details Used only by the public core error wrappers.
 */
#ifndef GENESIS_CORE_INTERNAL_ERROR_H
# define GENESIS_CORE_INTERNAL_ERROR_H
# pragma once

/* ************************************************************************** */
/*                                 Headers                                    */
/* ************************************************************************** */

/* -----| Internals |----- */
# include "../types.h"

/* ************************************************************************** */
/*                                 Prototypes                                 */
/* ************************************************************************** */

const char	*_core_error_strerror(
	const int	code
);
const char	*_genesis_strerror(
	const t_genesis_core_module	module,
	const int					code
);

#endif
