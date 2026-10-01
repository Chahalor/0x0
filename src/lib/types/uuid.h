/**
 * @file uuid.h
 * @brief In-memory UUID version 7 generation, parsing and formatting.
 * @details Generation reads the system clock and configured random source.
 * UUID allocations and returned strings are managed by lib/memory.
 */
#ifndef GENESIS_TYPES_UUID_UUID_H
# define GENESIS_TYPES_UUID_UUID_H
# pragma once

/* ----| Internals |----- */
# include "uuid/types.h"
# include "uuid/error.h"

/* ----| Macros     |----- */
# define UUID_STRING_SIZE 37
# define UUID_NULL ((UUID){{0}})
# define UUID_MAX ((UUID){{0xff, 0xff, 0xff, 0xff, \
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, \
	0xff, 0xff, 0xff, 0xff}})

/**
 * @brief Allocate a version 7 UUID using the applied default settings.
 * @return Owned UUID, or NULL with errno set to EINVAL, ENOMEM, EIO, or the
 * clock/random source error. Release with genesis_uuid_free().
 * @par Example
 * @code{.c}
 * UUID *id = genesis_uuid();
 * genesis_uuid_free(id);
 * @endcode
 */
UUID	*genesis_uuid(void);

/**
 * @brief Allocate a UUID of the requested version.
 * @param version Version number; currently only 7 is supported.
 * @return Owned UUID or NULL with errno EINVAL, ENOMEM, EIO, or a source
 * error. No allocation remains on failure.
 * @par Example
 * @code{.c}
 * UUID *id = genesis_uuid_generate(7);
 * genesis_uuid_free(id);
 * @endcode
 */
UUID	*genesis_uuid_generate(
	const int	version
);

/**
 * @brief Release an owned UUID from this module.
 * @param uuid UUID returned by genesis_uuid(), genesis_uuid_generate(), or
 * genesis_uuid_read(); NULL is accepted. Caller-owned UUID values must not
 * be passed here.
 * @return Nothing. The pointer is invalid afterward.
 * @par Example
 * @code{.c}
 * genesis_uuid_free(id);
 * @endcode
 */
void	genesis_uuid_free(
	UUID	*uuid
);

/**
 * @brief Replace a UUID value with a new version 7 value.
 * @param id Writable UUID; must not be NULL.
 * @return 0, EINVAL, EIO, a clock/random source error, or negative
 * UUID_ERROR_UNSUPPORTED_VERSION. The old value is unchanged on failure.
 * @par Example
 * @code{.c}
 * UUID id = UUID_NULL;
 * int status = genesis_uuid_set(&id);
 * @endcode
 */
int	genesis_uuid_set(
	UUID	*id
);

/**
 * @brief Parse a canonical 36-character UUID string into an owned UUID.
 * @param str Non-NULL canonical text with hyphens at positions 8, 13, 18, 23;
 * hex digits may be upper or lower case.
 * @return Owned UUID or NULL with errno EINVAL or ENOMEM. Invalid text leaves
 * no allocation behind. Release the result with genesis_uuid_free().
 * @par Example
 * @code{.c}
 * UUID *id = genesis_uuid_read("018f0e12-3456-7abc-8def-0123456789ab");
 * genesis_uuid_free(id);
 * @endcode
 */
UUID	*genesis_uuid_read(
	const char	*str
);

/**
 * @brief Parse text into an existing UUID value.
 * @param id Writable UUID; must not be NULL.
 * @param str Canonical text described by genesis_uuid_read(); not NULL.
 * @return 0, EINVAL for NULL arguments, or -UUID_ERROR_INVALID_FORMAT.
 * The old value remains unchanged on failure.
 * @par Example
 * @code{.c}
 * UUID id = UUID_NULL;
 * int status = genesis_uuid_read_set(&id, text);
 * @endcode
 */
int	genesis_uuid_read_set(
	UUID	*id,
	const char		*str
);

/**
 * @brief Compare UUID byte values in lexicographic order.
 * @param id1 First borrowed UUID; NULL sorts before non-NULL.
 * @param id2 Second borrowed UUID; NULL sorts before non-NULL.
 * @return Zero when equal, negative when id1 sorts first, positive otherwise.
 * Neither UUID changes.
 * @par Example
 * @code{.c}
 * int equal = genesis_uuid_cmp(&a, &b) == 0;
 * @endcode
 */
int	genesis_uuid_cmp(
	const UUID	*id1,
	const UUID	*id2
);

/**
 * @brief Read the version nibble from a UUID value.
 * @param id Borrowed UUID, or NULL.
 * @return Integer version 0 to 15; INT_MAX for NULL. No state changes.
 * @par Example
 * @code{.c}
 * int version = genesis_uuid_version(&id);
 * @endcode
 */
int	genesis_uuid_version(
	const UUID	*id
);

/**
 * @brief Render a UUID as lower-case canonical text.
 * @param id Borrowed UUID, or NULL to render the nil UUID.
 * @return Owned 37-byte NUL-terminated string, or NULL with errno ENOMEM.
 * Release the result with mem_free(). The UUID is unchanged.
 * @par Example
 * @code{.c}
 * char *text = genesis_uuid_stringify(&id);
 * mem_free(text);
 * @endcode
 */
char	*genesis_uuid_stringify(
	const UUID	*id
);

#endif
