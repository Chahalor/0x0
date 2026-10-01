/**
 * 
*/
/* ----| Headers    |----- */
	/* Standard */
		//...

	/* Internal */
#include "_uuid.h"

	/* External */
		//...

/* ----| Prototypes |----- */
	//...

/* ----| Internals  |----- */
	//...

/* ----| Public     |----- */

UUID	*uuid_new_(void)
{
	return (G_SETTINGS.allocator.alloc(sizeof(UUID)));
}

void	uuid_free_(
	UUID	*const restrict	uuid
)
{
	G_SETTINGS.allocator.free(uuid);
}