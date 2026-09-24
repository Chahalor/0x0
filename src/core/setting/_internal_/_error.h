/**
 * @file _error.h
 * @brief Private error message lookup for the setting module.
 * @details Used only by its public error wrapper.
 */
#ifndef GENESIS_CORE_SETTING_INTERNAL_ERROR_H
# define GENESIS_CORE_SETTING_INTERNAL_ERROR_H
# pragma once

/* ************************************************************************** */
/*                                 Prototypes                                 */
/* ************************************************************************** */

const char	*_setting_strerror(
	const int	code
);

#endif
