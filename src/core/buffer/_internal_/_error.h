/**
 * @file _error.h
 * @brief Private error message lookup for the buffer module.
 * @details Used only by its public error wrapper.
 */
#ifndef GENESIS_CORE_BUFFER_INTERNAL_ERROR_H
# define GENESIS_CORE_BUFFER_INTERNAL_ERROR_H
# pragma once

/* ************************************************************************** */
/*                                 Prototypes                                 */
/* ************************************************************************** */

const char	*_buffer_strerror(
	const int	code
);

#endif
