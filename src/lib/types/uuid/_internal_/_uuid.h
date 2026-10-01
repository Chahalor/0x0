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
# include "core/setting/setting.h"

/* -----| Internals |----- */
# include "_types.h"
# include "../../error.h"
# include "../uuid.h"

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

UUID	*uuid_new_(void);

void	uuid_free_(
	UUID	*const restrict	uuid
);

int	set_uuid_v7(
	UUID	*const	uuid
);

int	set_uuid_(
	UUID	*const	uuid,
	const int		version
);

UUID	*genesis_uuid_generate_(
	const int	version
);


int	uuid_parse_(
	UUID		*const	dest,
	const char	*const	str
);

void	uuid_fill_string_(
	const UUID	*const	uuid,
	char		*const	r
);

char	*uuid_stringify_(
	const UUID	*const	uuid
);