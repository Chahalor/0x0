/**
 * @file setting.h
 * @brief Runtime behavior settings for UUID generation.
 * @details Public edits take effect after apply succeeds. The private active
 * configuration remains unchanged when validation fails.
 */
#ifndef GENESIS_TYPES_UUID_SETTING_H
# define GENESIS_TYPES_UUID_SETTING_H
# pragma once

/* ----| Systems   |----- */
# include <stddef.h>
# include <sys/types.h>

/* ----| Typedefs  |----- */
typedef struct s_genesis_uuid_setting	t_genesis_uuid_setting;

/* ----| Structs   |----- */
struct s_genesis_uuid_setting
{
	int			default_version;
	ssize_t		(*getrandom)(void *, size_t, unsigned int);
	int			(*apply)(void);
};

/**
 * @brief Mutable UUID settings workspace.
 * @details Starts with version 7 and the system getrandom callback. A callback
 * may return fewer bytes than requested; generation retries until 16 bytes
 * are received. Synchronize edits and apply with concurrent UUID operations.
 * @par Example
 * @code{.c}
 * genesis_uuid_setting.getrandom = custom_random;
 * genesis_uuid_setting.apply();
 * @endcode
 */
extern t_genesis_uuid_setting	genesis_uuid_setting;

/**
 * @brief Validate and publish the public UUID settings without arguments.
 * @return 0 on success, or EINVAL unless the version is 7 and getrandom is
 * non-NULL. Failure keeps the previous active settings.
 * @par Example
 * @code{.c}
 * genesis_uuid_setting.default_version = 7;
 * int status = genesis_uuid_settings_apply();
 * @endcode
 */
int	genesis_uuid_settings_apply(void);

#endif
