// Header

/* ----| Headers    |----- */
	/* Standard */
		//...

	/* Internal */
#include "_internal_/_memory.h"
#include "memory.h"
#include <errno.h>

	/* External */
		//...

/* ----| Prototypes |----- */
	//...

/* ----| Internals  |----- */
	//...

/* ----| Public     |----- */

void	*mem_alloc(
	const size_t	_size
)
{
	void	*result = NULL;

	result = mem_manager_(mem_manager_access_alloc, NULL, _size);

	return (result);
}

void	*mem_realloc(
	void			*_ptr,
	const size_t	_size
)
{
	void	*result = NULL;

	result = mem_manager_(mem_manager_access_realloc, _ptr, _size);

	return (result);
}

void	*mem_dup(
	const void *const restrict _ptr,
	const ssize_t _size
)
{
	void	*result = NULL;

	result = mem_manager_(mem_manager_access_dup, _ptr, _size);

	return (result);
}

void	mem_free(
	void *const restrict _ptr
)
{
	(void)mem_manager_(mem_manager_access_free, _ptr, 0);
}

__attribute__((destructor))
void	mem_free_all(void)
{
	mem_manager_(mem_manager_access_free_all, NULL, 0);
}

void	mem_manager_rm(
	void	*const	ptr
)
{
	if (unlikely(!ptr))
		return ;

	(void)mem_manager_(mem_manager_access_remove, ptr, 0);
}

MEM_ARENA	*mem_arena_new(void)
{
	return (arena_new_());
}

void	mem_arena_free(
	MEM_ARENA *const arena
)
{
	if (arena)
		arena_free_(arena);
}

void	mem_arena_reset(
	MEM_ARENA *const arena
)
{
	if (arena)
		arena_reset_(arena);
}

void	*mem_arena_alloc(
	MEM_ARENA *const	arena,
	const size_t		size
)
{
	if (!arena || size == 0)
	{
		errno = EINVAL;
		return (NULL);
	}

	return (arena_alloc_(arena, size));
}

void	*mem_arena_dup(
	MEM_ARENA *const	arena,
	void *const			ptr,
	const ssize_t		size
)
{
	if (!arena || !ptr || size < -1 || size == 0)
	{
		errno = EINVAL;
		return (NULL);
	}

	return (arena_dup_(arena, ptr, size));
}

t_arena_mark	mem_arena_mark(
	MEM_ARENA *const	arena
)
{
	if (!arena)
	{
		errno = EINVAL;
		return (NULL);
	}

	return (arena_mark_(arena));
}

void	mem_arena_rewind(
	MEM_ARENA *const	arena,
	const t_arena_mark	mark
)
{
	if (!arena || !mark)
	{
		errno = EINVAL;
		return ;
	}

	arena_rewind_(arena, mark);
}

void	mem_arena_trim(
	MEM_ARENA *const	arena
)
{
	if (arena)
		arena_trim_(arena);
}
