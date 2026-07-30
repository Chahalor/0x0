/**
 * 
*/

#ifndef GENESIS_NETWORK_PROTOCOLS_HTTP_ERROR_H
# define GENESIS_NETWORK_PROTOCOLS_HTTP_ERROR_H

# pragma once

// Header

# pragma once

/* ************************************************************************** */
/*                                 Headers                                    */
/* ************************************************************************** */

/* -----| Systems   |----- */
	//...

/* -----| Globals   |----- */
# include "lib_config.h"
# include "lib_standards.h"

/* -----| Internals |----- */
# include "types.h"

/* -----| Modules   |----- */
	//...

/* ************************************************************************** */
/*                                 Enums                                      */
/* ************************************************************************** */

enum	e_net_prot_http_errors
{
	HTTP_NO_ERROR = error_none,
	HTTP_ERROR_SYSCALL,
	HTTP_ERROR_BAD_REQUEST,
};

/* ************************************************************************** */
/*                                 Prototypes                                 */
/* ************************************************************************** */

int			net_prot_http_errno(void);
const char	*net_prot_http_strerror(const int code);

#endif	// GENESIS_NETWORK_PROTOCOLS_HTTP_ERROR_H