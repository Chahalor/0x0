/**
 * @file _arena.c
 * @brief Fixed-capacity arenas owned by the memory manager.
 */

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "_memory.h"

#define ARENA_DEFAULT_CAPACITY (64u * 1024u)

static int arena_known__(const MEM_ARENA *arena)
{
	const MEM_ARENA *current;

	current = mem_manager_instance_()->arenas;
	while (current)
	{
		if (current == arena)
			return (1);
		current = current->next;
	}
	return (0);
}

static void arena_clear_marks__(MEM_ARENA *arena)
{
	t_mem_arena_mark *mark;
	t_mem_arena_mark *next;

	mark = arena->marks;
	while (mark)
	{
		next = mark->next;
		G_SETTINGS.allocator.free(mark);
		mark = next;
	}
	mark = arena->retired_marks;
	while (mark)
	{
		next = mark->next;
		G_SETTINGS.allocator.free(mark);
		mark = next;
	}
	arena->marks = NULL;
	arena->retired_marks = NULL;
	arena->stats.nb_mark = 0;
}

static void arena_retire_marks__(MEM_ARENA *arena)
{
	t_mem_arena_mark *mark;
	t_mem_arena_mark *next;

	mark = arena->marks;
	while (mark)
	{
		next = mark->next;
		mark->next = arena->retired_marks;
		arena->retired_marks = mark;
		mark = next;
	}
	arena->marks = NULL;
	arena->stats.nb_mark = 0;
}

static void arena_release__(MEM_ARENA *arena)
{
	arena_clear_marks__(arena);
	if (arena->bytes)
		G_SETTINGS.allocator.free(arena->bytes);
	G_SETTINGS.allocator.free(arena);
}

MEM_ARENA *arena_new_(void)
{
	t_mem_manager *manager;
	MEM_ARENA *arena;

	if (!G_SETTINGS.allocator.alloc || !G_SETTINGS.allocator.free)
	{
		errno = EINVAL;
		return (NULL);
	}
	arena = G_SETTINGS.allocator.alloc(sizeof(*arena));
	if (!arena)
	{
		errno = ENOMEM;
		return (NULL);
	}
	memset(arena, 0, sizeof(*arena));
	arena->bytes = G_SETTINGS.allocator.alloc(ARENA_DEFAULT_CAPACITY);
	if (!arena->bytes)
	{
		G_SETTINGS.allocator.free(arena);
		errno = ENOMEM;
		return (NULL);
	}
	arena->capacity = ARENA_DEFAULT_CAPACITY;
	manager = mem_manager_instance_();
	arena->next = manager->arenas;
	manager->arenas = arena;
	return (arena);
}

void arena_free_(MEM_ARENA *arena)
{
	MEM_ARENA **link;

	link = &mem_manager_instance_()->arenas;
	while (*link && *link != arena)
		link = &(*link)->next;
	if (!*link)
	{
		errno = EINVAL;
		return ;
	}
	*link = arena->next;
	arena_release__(arena);
}

void arena_reset_(MEM_ARENA *arena)
{
	if (!arena_known__(arena))
	{
		errno = EINVAL;
		return ;
	}
	if (arena->bytes)
		memset(arena->bytes, 0, arena->used);
	arena->used = 0;
	arena->stats.nb_block = 0;
	arena_retire_marks__(arena);
}

void *arena_alloc_(MEM_ARENA *arena, size_t size)
{
	const size_t alignment = _Alignof(max_align_t);
	size_t padding;
	size_t offset;
	void *result;

	if (!arena_known__(arena))
	{
		errno = EINVAL;
		return (NULL);
	}
	padding = (alignment - arena->used % alignment) % alignment;
	if (size > SIZE_MAX - padding)
	{
		errno = EOVERFLOW;
		return (NULL);
	}
	if (padding > arena->capacity - arena->used
		|| size > arena->capacity - arena->used - padding)
	{
		errno = ENOMEM;
		return (NULL);
	}
	if (!arena->bytes)
	{
		if (!G_SETTINGS.allocator.alloc)
		{
			errno = EINVAL;
			return (NULL);
		}
		arena->bytes = G_SETTINGS.allocator.alloc(arena->capacity);
		if (!arena->bytes)
		{
			errno = ENOMEM;
			return (NULL);
		}
	}
	offset = arena->used + padding;
	result = arena->bytes + offset;
	memset(result, 0, size);
	arena->used = offset + size;
	arena->stats.nb_block++;
	return (result);
}

void *arena_dup_(MEM_ARENA *arena, void *ptr, ssize_t size)
{
	size_t tracked;
	size_t count;
	void *result;

	tracked = mem_manager_size_(ptr);
	if (size == -1)
	{
		if (!tracked)
		{
			errno = EINVAL;
			return (NULL);
		}
		count = tracked;
	}
	else
	{
		count = (size_t)size;
		if (tracked && count > tracked)
		{
			errno = EINVAL;
			return (NULL);
		}
	}
	result = arena_alloc_(arena, count);
	if (result)
		memmove(result, ptr, count);
	return (result);
}

t_arena_mark arena_mark_(MEM_ARENA *arena)
{
	t_mem_arena_mark *mark;

	if (!arena_known__(arena))
	{
		errno = EINVAL;
		return (NULL);
	}
	if (arena->stats.nb_mark == UINT_MAX)
	{
		errno = EOVERFLOW;
		return (NULL);
	}
	if (!G_SETTINGS.allocator.alloc)
	{
		errno = EINVAL;
		return (NULL);
	}
	mark = G_SETTINGS.allocator.alloc(sizeof(*mark));
	if (!mark)
	{
		errno = ENOMEM;
		return (NULL);
	}
	mark->offset = arena->used;
	mark->blocks = arena->stats.nb_block;
	mark->next = arena->marks;
	arena->marks = mark;
	arena->stats.nb_mark++;
	return (mark);
}

void arena_rewind_(MEM_ARENA *arena, t_arena_mark token)
{
	t_mem_arena_mark *mark;
	t_mem_arena_mark *next;

	if (!arena_known__(arena))
	{
		errno = EINVAL;
		return ;
	}
	mark = arena->marks;
	while (mark && mark != token)
		mark = mark->next;
	if (!mark)
	{
		errno = EINVAL;
		return ;
	}
	while (arena->marks != mark)
	{
		next = arena->marks->next;
		arena->marks->next = arena->retired_marks;
		arena->retired_marks = arena->marks;
		arena->marks = next;
		arena->stats.nb_mark--;
	}
	if (arena->bytes)
		memset(arena->bytes + mark->offset, 0, arena->used - mark->offset);
	arena->used = mark->offset;
	arena->stats.nb_block = mark->blocks;
}

void arena_trim_(MEM_ARENA *arena)
{
	if (!arena_known__(arena))
	{
		errno = EINVAL;
		return ;
	}
	if (arena->used == 0 && arena->bytes)
	{
		G_SETTINGS.allocator.free(arena->bytes);
		arena->bytes = NULL;
	}
}

void mem_arena_release_all_(t_mem_manager *manager)
{
	MEM_ARENA *arena;
	MEM_ARENA *next;

	arena = manager->arenas;
	manager->arenas = NULL;
	while (arena)
	{
		next = arena->next;
		arena_release__(arena);
		arena = next;
	}
}
