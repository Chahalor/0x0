/**
 * 
*/
/* ----| Headers    |----- */
	/* Standard */
#include <string.h>

	/* Internal */
#include "_uuid.h"

	/* External */
		//...

/* ----| Prototypes |----- */
	//...

/* ----| Internals  |----- */
	//...

/* ----| Public     |----- */

int	uuid_version_(
	const UUID	*const	uuid
)
{
	return (uuid->bytes[6] >> 4);
}

int	uuid_cmp_(
	const UUID	*const	id1,
	const UUID	*const	id2
)
{
	return (memcmp(id1->bytes, id2->bytes, sizeof(id1->bytes)));
}