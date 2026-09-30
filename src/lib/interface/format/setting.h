/**
 * @file setting.h
 * @brief Public workspace for format module settings.
 * @details The application may change genesis_format_settings freely. Runtime
 * code uses only the private configuration published by apply().
 */
#ifndef GENESIS_INTERFACE_FORMAT_SETTING_H
# define GENESIS_INTERFACE_FORMAT_SETTING_H
# pragma once

/* -----| Systems   |----- */
# include <stddef.h>

/* -----| Typedefs  |----- */
typedef struct s_genesis_format_settings	t_genesis_format_settings;

/* -----| Structs   |----- */
/**
 * @brief Mutable public format settings workspace.
 * @details max_output counts produced bytes excluding NUL; zero permits only
 * empty output. Call apply() after edits to publish a validated snapshot.
 */
struct s_genesis_format_settings
{
	size_t						max_output;
	int							(*apply)(void);
};

/**
 * @brief Public format settings workspace, initialized to module defaults.
 * @details Direct edits have no runtime effect until apply() succeeds.
 * Synchronize edits and apply calls with other threads using the module.
 * Results are owned by lib/memory and can be released with mem_free().
 * @par Example
 * @code{.c}
 * genesis_format_settings.max_output = 1024;
 * genesis_format_settings.apply();
 * @endcode
 */
extern t_genesis_format_settings	genesis_format_settings;

/**
 * @brief Validate and apply the public format settings without arguments.
 * @return 0 on success or EINVAL for an invalid output limit.
 * On failure the active internal configuration remains unchanged. Subsequent
 * formatting calls use the new limit only after success.
 * Synchronize with concurrent edits and module calls.
 * @par Example
 * @code{.c}
 * genesis_format_settings.max_output = 1024;
 * int status = genesis_format_settings_apply();
 * @endcode
 */
int	genesis_format_settings_apply(void);

#endif
