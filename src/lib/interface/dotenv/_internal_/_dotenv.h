#ifndef GENESIS_INTERFACE_DOTENV_INTERNAL_DOTENV_H
# define GENESIS_INTERFACE_DOTENV_INTERNAL_DOTENV_H

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
#include "../settings.h"
# include "./_types.h"
#include "../../dotenv.h"

/* -----| Modules   |----- */
#include "memory.h"

/* ************************************************************************** */
/*                                 Macros                                     */
/* ************************************************************************** */

# define	_NEEDLE_MAX_LEN	128
# define	SETTINGS		g_dotenv_settings

/* ************************************************************************** */
/*                                 Prototypes                                 */
/* ************************************************************************** */

t_interface_dotenv_error	_dotenv_load_file(
	const char	*const	_path
);

#endif	// GENESIS_INTERFACE_DOTENV_INTERNAL_DOTENV_H
