/**
 * @file uuid.c
 * @brief Public validation and dispatch for UUID operations.
 */

/* ----| Headers    |----- */
	/* Standard */
# include <errno.h>
# include <limits.h>

	/* Internal */
# include "../uuid.h"
# include "_internal_/_uuid.h"

/* ----| Public     |----- */
UUID	*genesis_uuid(void)
{
	return (genesis_uuid_generate_(uuid_config.default_version));
}

UUID	*genesis_uuid_generate(
	int version
)
{
	return (genesis_uuid_generate_(version));
}

void	genesis_uuid_free(
	UUID *uuid
)
{
	uuid_free_(uuid);
}

int	genesis_uuid_set(
	UUID *id
)
{
	if (!id)
		return (EINVAL);
	return (set_uuid_(id, uuid_config.default_version));
}

UUID	*genesis_uuid_read(
	const char *str
)
{
	if (!str)
	{
		errno = EINVAL;
		return (NULL);
	}
	return (_uuid_read(str));
}

int	genesis_uuid_read_set(
	UUID *id,
	const char *str
)
{
	if (!id || !str)
		return (EINVAL);
	return (uuid_parse_(id, str));
}

int	genesis_uuid_cmp(
	const UUID *id1,
	const UUID *id2
)
{
	return (_uuid_compare(id1, id2));
}

int	genesis_uuid_version(
	const UUID *id
)
{
	if (!id)
		return (INT_MAX);
	return (uuid_version_(id));
}

char	*genesis_uuid_stringify(
	const UUID *id
)
{
	return (_uuid_stringify(id));
}
