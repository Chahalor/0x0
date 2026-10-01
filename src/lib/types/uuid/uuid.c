// Header

/* ----| Headers    |----- */
	/* Standard */
		//...

	/* Internal */
#include "uuid.h"
#include "_internal_/_uuid.h"

	/* External */
		//...

/* ----| Prototypes |----- */
	//...

/* ----| Internals  |----- */
	//...

/* ----| Public     |----- */

UUID	*genesis_uuid(void)
{
	return (genesis_uuid_generate_(uuid_config.default_version));
}

UUID	*genesis_uuid_generate(
	const int	version
)
{
	return (genesis_uuid_generate_(version));
}

int		genesis_uuid_set(
	UUID	*const restrict	id
)
{
	if (unlikely(!id))
		return (EINVAL);

	return (set_uuid_(id, uuid_config.default_version));
}

UUID	*genesis_uuid_read(
	const char	*const	str
)
{
	UUID	*r;

	if (unlikely(!str))
		return (NULL);
	
	r = uuid_new_();
	if (unlikely(!r))
		return (EINVAL);

	return (uuid_parse_(r, str));
}

int	genesis_uuid_read_set(
	UUID		*const restrict	id,
	const char	*const			str
)
{
	if (unlikely(!id || !str))
		return (EINVAL);

	return (uuid_parse_(id, str));
}

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
)
{
	static const UUID	NILL = UUID_NULL;

	if (unlikely(!id))
		return (uuid_stringify_(&NILL));

	return (uuid_stringify_(id));
}