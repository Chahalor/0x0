/**
 * @file _types.h
 * @brief Private linked entry type for the settings manager.
 * @details Used only inside the core setting module.
 */
#ifndef GENESIS_CORE_SETTING_INTERNAL_TYPES_H
# define GENESIS_CORE_SETTING_INTERNAL_TYPES_H
# pragma once

/* ************************************************************************** */
/*                                 Headers                                    */
/* ************************************************************************** */

/* -----| Internals |----- */
# include "../types.h"

/* ************************************************************************** */
/*                                 Structs                                    */
/* ************************************************************************** */

struct s_genesis_setting_node
{
	const t_genesis_setting_key	*key;
	t_genesis_setting_node		*next;
	t_genesis_core_allocator	allocator;
	unsigned char				value[];
};

#endif
