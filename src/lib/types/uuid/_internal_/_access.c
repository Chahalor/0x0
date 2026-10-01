/**
 * @file _access.c
 * @brief Access UUID version and compare byte values.
 */

/* ----| Headers    |----- */
	/* Standard */
# include <string.h>

	/* Internal */
# include "_uuid.h"

/* ----| Internals  |----- */
/** @brief Read the version nibble from a non-NULL UUID. */
int uuid_version_(
	const UUID *uuid
)
{
	return (uuid->bytes[6] >> 4);
}

/** @brief Compare the complete byte representation of two UUIDs. */
int uuid_cmp_(
	const UUID *id1,
	const UUID *id2
)
{
	return (memcmp(id1->bytes, id2->bytes, sizeof(id1->bytes)));
}
