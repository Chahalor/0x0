/**
 * @file _generate.c
 * @brief Generate UUID version 7 values from time and configured randomness.
 */

/* ----| Headers    |----- */
	/* Standard */
# include <errno.h>
# include <stdint.h>
# include <time.h>

	/* Internal */
# include "_uuid.h"

/* ----| Internals  |----- */
/** @brief Build a v7 value only after clock and random input succeed. */
int set_uuid_v7(
	UUID *uuid
)
{
	UUID	result;
	struct timespec	now;
	uint64_t		milliseconds;
	size_t			offset;
	ssize_t			count;
	size_t			i;

	if (!uuid)
		return (EINVAL);

	if (clock_gettime(CLOCK_REALTIME, &now) != 0)
		return (errno ? errno : EIO);

	milliseconds = (uint64_t)now.tv_sec * 1000u + (uint64_t)now.tv_nsec / 1000000u;
	offset = 0;
	while (offset < sizeof(result.bytes))
	{
		count = uuid_config.getrandom(result.bytes + offset,
			sizeof(result.bytes) - offset, 0);
		if (count < 0 && errno == EINTR)
			continue;

		if (count <= 0)
			return (count == 0 ? EIO : (errno ? errno : EIO));

		if ((size_t)count > sizeof(result.bytes) - offset)
			return (EIO);

		offset += (size_t)count;
	}
	for (i = 0; i < 6; ++i)
		result.bytes[i] = (uint8_t)(milliseconds >> (40u - 8u * i));

	result.bytes[6] = (uint8_t)((result.bytes[6] & 0x0fu) | 0x70u);
	result.bytes[8] = (uint8_t)((result.bytes[8] & 0x3fu) | 0x80u);
	*uuid = result;
	return (0);
}

/** @brief Dispatch generation for the supported UUID version. */
int set_uuid_(
	UUID *uuid,
	int version
)
{
	if (!uuid)
		return (EINVAL);

	if (version != 7)
		return (-UUID_ERROR_UNSUPPORTED_VERSION);

	return (set_uuid_v7(uuid));
}

/** @brief Allocate a generated UUID and release it on any failure. */
UUID *genesis_uuid_generate_(
	int version
)
{
	UUID	*uuid;
	int				status;

	if (version != 7)
	{
		errno = EINVAL;
		return (NULL);
	}

	uuid = uuid_new_();
	if (!uuid)
		return (NULL);

	status = set_uuid_(uuid, version);
	if (status != 0)
	{
		uuid_free_(uuid);
		errno = status > 0 ? status : EINVAL;
		return (NULL);
	}

	return (uuid);
}
