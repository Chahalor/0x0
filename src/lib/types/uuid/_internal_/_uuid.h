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
# include "_types.h"
# include "../../error.h"

/* -----| Modules   |----- */
	//...

/* ************************************************************************** */
/*                                 Macros                                     */
/* ************************************************************************** */

extern t_genesis_uuid_config	uuid_config = {
	.default_version	= 7
};

/* ************************************************************************** */
/*                                 Prototypes                                 */
/* ************************************************************************** */

int	set_uuid_v7(
	UUID	*const	uuid
);

int	set_uuid_(
	UUID	*const	uuid,
	const int		version
);

UUID	*genesis_uuid_(void);

UUID	*genesis_uuid_generate_(
	const int	version
);
