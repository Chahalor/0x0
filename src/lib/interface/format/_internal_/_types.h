/**
 * @file _types.h
 * @brief Private state of one format operation and its current specifier.
 * @details Private only; depends on the public format callback type.
 */
#ifndef GENESIS_INTERFACE_FORMAT_INTERNAL_TYPES_H
# define GENESIS_INTERFACE_FORMAT_INTERNAL_TYPES_H
# pragma once

/* -----| Systems   |----- */
# include <stdarg.h>
# include <stdbool.h>
# include <stddef.h>
# include <stdint.h>

/* -----| Internals |----- */
# include "../types.h"

/* -----| Typedefs  |----- */
typedef struct s_genesis_format_spec_	t_spec;
typedef struct s_genesis_format_		t_genesis_format_;

typedef struct s_format_tag_entry	t_format_tag_entry;
typedef struct s_format_registry	t_format_registry;
typedef struct s_format_allocator	t_format_allocator;


/* -----| Structs   |----- */
struct s_genesis_format_spec_
{
	char		text[64];
	size_t		size;
	char		conversion;
	char		length[3];
	unsigned int	width;
	int			precision;
	bool		left;
	bool		plus;
	bool		space;
	bool		zero;
	bool		alternate;
};

struct s_genesis_format_
{
	const char					*fm;
	char						*buffer;
	size_t						capacity;
	size_t						index;
	size_t						cursor;
	bool						fixed_capacity;
	va_list						list;
	size_t						max_output;
	t_spec						current;
};

struct s_format_allocator
{
	void *(*alloc)(size_t);
	void (*free)(void *);
};

struct s_format_tag_entry
{
	t_format_tag_entry			*next;
	char						*tag;
	size_t						length;
	uint64_t					hash;
	t_format_callback			callback;
	t_format_allocator	allocator;
};

struct s_format_registry
{
	t_format_tag_entry			**buckets;
	size_t						bucket_count;
	size_t						tag_count;
	t_format_allocator	bucket_allocator;
};

#endif
