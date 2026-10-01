/**
 * @file _types.h
 * @brief Private snapshot of applied UUID behavior settings.
 */
#ifndef GENESIS_TYPES_UUID_INTERNAL_TYPES_H
# define GENESIS_TYPES_UUID_INTERNAL_TYPES_H
# pragma once

/* ----| System    |----- */
# include <stdint.h>

/* ----| Internals |----- */
# include "../setting.h"

/* ----| Typedefs  |----- */
typedef struct s_genesis_uuid_config	t_genesis_uuid_config;

/* ----| Structs   |----- */
struct s_genesis_uuid_config
{
	int			default_version;
	ssize_t		(*getrandom)(void *, size_t, unsigned int);
};

struct s_genesis_uuid
{
	uint8_t	bytes[16];
};

#endif
