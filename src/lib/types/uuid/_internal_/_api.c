/**
 * @file _api.c
 * @brief Ownership and NULL semantics for UUID operations.
 */

/* ----| Headers    |----- */
	/* Standard */
# include <errno.h>

	/* Internal */
# include "_uuid.h"

/* ----| Internals  |----- */
/** @brief Allocate, parse, and release on failure. */
UUID *_uuid_read(
	const char *str
)
{
	UUID *uuid;
	int status;

	uuid = uuid_new_();
	if (!uuid)
		return (NULL);

	status = uuid_parse_(uuid, str);
	if (status != 0)
	{
		uuid_free_(uuid);
		errno = status > 0 ? status : EINVAL;
		return (NULL);
	}

	return (uuid);
}

/** @brief Define a total order including NULL values. */
int _uuid_compare(
	const UUID *id1,
	const UUID *id2
)
{
	if (!id1 || !id2)
		return (id1 == id2
			? 0
			: (id1
				? 1
				: -1
			)
		);

	return (uuid_cmp_(id1, id2));
}

/** @brief Render NULL as the nil UUID. */
char *_uuid_stringify(
	const UUID *id
)
{
	static const UUID nil = {{0}};

	return (uuid_stringify_(id ? id : &nil));
}
