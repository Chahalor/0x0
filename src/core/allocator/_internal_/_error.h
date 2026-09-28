/**
 * @file _error.h
 * @brief Private error message lookup for the allocator module.
 * @details Used only by its public error wrapper.
 */
#ifndef GENESIS_CORE_ALLOCATOR_INTERNAL_ERROR_H
# define GENESIS_CORE_ALLOCATOR_INTERNAL_ERROR_H
# pragma once

/* ************************************************************************** */
/*                                 Prototypes                                 */
/* ************************************************************************** */

const char	*_allocator_strerror(
	const int	code
);

#endif
