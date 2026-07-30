#ifndef GENESIS_NETWORK_COMMUNICATION_INTERNAL_SERVER_SERVER_H
# define GENESIS_NETWORK_COMMUNICATION_INTERNAL_SERVER_SERVER_H

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
# include "../_network.h"

/* -----| Modules   |----- */
	//...

/* ************************************************************************** */
/*                                 Macros                                     */
/* ************************************************************************** */
	//...

/* ************************************************************************** */
/*                                 Prototypes                                 */
/* ************************************************************************** */

int	_net_server_start(
		t_net_conn *const restrict _server,
		const int _port
		);

int	_net_server_accept(
		t_net_conn *const restrict _server
		);

int	_net_server_close(
		t_net_conn *const restrict _server
		);

#endif	// GENESIS_NETWORK_COMMUNICATION_INTERNAL_SERVER_SERVER_H
