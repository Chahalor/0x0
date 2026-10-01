/**
 * @file _memory.c
 * @brief Allocate UUID values through the Genesis memory module.
 */

/* ----| Headers    |----- */
	/* Internal */
# include "_uuid.h"

	/* Modules */
# include "memory.h"

/* ----| Internals  |----- */
/** @brief Allocate a tracked UUID value. */
UUID *uuid_new_(void)
{
	return (mem_alloc(sizeof(UUID)));
}

/** @brief Release a tracked UUID value. */
void uuid_free_(
	UUID *uuid
)
{
	mem_free(uuid);
}
