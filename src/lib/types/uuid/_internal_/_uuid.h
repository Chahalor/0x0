/**
 * @file _uuid.h
 * @brief Private UUID operations and active settings.
 */
#ifndef GENESIS_TYPES_UUID_INTERNAL_H
# define GENESIS_TYPES_UUID_INTERNAL_H
# pragma once

/* ----| Internals |----- */
# include "_types.h"
# include "../../uuid.h"

/* ----| State      |----- */
extern t_genesis_uuid_config	uuid_config;

/* ----| Prototypes |----- */
int			_uuid_settings_apply(void);
const char	*_uuid_strerror(int code);
UUID		*uuid_new_(void);
void		uuid_free_(UUID *uuid);
int			set_uuid_v7(UUID *uuid);
int			set_uuid_(UUID *uuid, int version);
UUID		*genesis_uuid_generate_(int version);
UUID		*_uuid_read(const char *str);
int			uuid_parse_(UUID *dest, const char *str);
void		uuid_fill_string_(const UUID *uuid, char *result);
char		*uuid_stringify_(const UUID *uuid);
char		*_uuid_stringify(const UUID *uuid);
int			uuid_version_(const UUID *uuid);
int			uuid_cmp_(const UUID *id1, const UUID *id2);
int			_uuid_compare(const UUID *id1, const UUID *id2);

#endif
