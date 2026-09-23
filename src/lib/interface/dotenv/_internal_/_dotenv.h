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
// # include "types.h"

/* -----| Modules   |----- */
#include "../../dotenv.h"
#include "memory.h"

/* ************************************************************************** */
/*                                 Macros                                     */
/* ************************************************************************** */

#define _NEEDLE_MAX_LEN	128

/* ************************************************************************** */
/*                                 Prototypes                                 */
/* ************************************************************************** */

int	_dotenv_load_file(
	const char *const _path
);

#endif	// GENESIS_INTERFACE_DOTENV_INTERNAL_DOTENV_H
