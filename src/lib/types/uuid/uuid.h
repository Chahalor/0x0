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
# include "../types.h"

/* -----| Modules   |----- */
	//...

/* ************************************************************************** */
/*                                 Macros                                     */
/* ************************************************************************** */
	//...

/* ************************************************************************** */
/*                                 Prototypes                                 */
/* ************************************************************************** */

// MARK: generate

UUID	*genesis_uuid(void);
UUID	*genesis_uuid_generate(
	const int	version
);

int		genesis_uuid_set(
	UUID	*const restrict	id
);

UUID	*genesis_uuid_read(
	const char	*const	str
);

int	genesis_uuid_read_set(
	UUID		*const restrict	id,
	const char	*const			str
);

// MARK: usage

int	genesis_uuid_cmp(
	const UUID	*const	id1,
	const UUID	*const	id2
);

int	genesis_uuid_version(
	const UUID	*const	id
);

// MARK: strings

char	*genesis_uuid_stringify(
	const UUID	*const	id
);