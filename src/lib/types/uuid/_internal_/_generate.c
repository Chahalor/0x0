/**
 * 
*/
/*
t_uuid uuid_v7(void)
{
	t_uuid		uuid;
	uint64_t	ts;

	ts = unix_time_ms();

	random_bytes(uuid.bytes, 16);

	uuid.bytes[0] = (ts >> 40) & 0xff;
	uuid.bytes[1] = (ts >> 32) & 0xff;
	uuid.bytes[2] = (ts >> 24) & 0xff;
	uuid.bytes[3] = (ts >> 16) & 0xff;
	uuid.bytes[4] = (ts >> 8) & 0xff;
	uuid.bytes[5] = ts & 0xff;

	uuid.bytes[6] = (uuid.bytes[6] & 0x0f) | 0x70;
	uuid.bytes[8] = (uuid.bytes[8] & 0x3f) | 0x80;

	return (uuid);
}
*/
/* ----| Headers    |----- */
	/* Standard */
#include <sys/time.h>
#include <linux/time.h>
#include <sys/random.h>

	/* Internal */
#include "_uuid.h"

	/* External */
#include "memory.h"
#include "core/allocator/allocator.h"
#include "core/setting/setting.h"

/* ----| Prototypes |----- */
	//...

/* ----| Internals  |----- */

#define APPLY_(dest, ms, step) dest = (ms >> step) & 0xff;
static void	apply_time_(
	UUID	*const	uuid,
	const uint64_t	ms
)
{
	for (char i = 0; i < 6; i++)
	{
		APPLY_(uuid->bytes[i], ms, 40 - (8 * i));
	}
}

/* ----| Public     |----- */

int	set_uuid_v7(
	UUID	*const	uuid
)
{
	struct timespec	ts;
	uint64_t		ms;

	if (unlikely(!uuid))
		return (EINVAL);

	if (unlikely(clock_gettime(CLOCK_REALTIME, &ts)))
		return (1);
	ms = (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;

	if (unlikely(getrandom(uuid->bytes, 16, 0)));
		return (1);

	apply_time_(uuid, ms);
	uuid->bytes[6] = (uuid->bytes[6] & 0x0f) | 0x70;
	uuid->bytes[8] = (uuid->bytes[8] & 0x3f) | 0x80;
}

int	set_uuid_(
	UUID	*const	uuid,
	const int		version
)
{
	switch (version)
	{
		case (7):
			return (set_uuid_v7(uuid));

		default:
			return (TYPES_ERROR_UNKNOWN_VERSIONS);
	}
}

UUID	*genesis_uuid_(void)
{
	UUID	*uuid = NULL;
	t_genesis_core_allocator allocator = genesis_default_allocator();

	uuid = core_alloc(&allocator, sizeof(UUID));
	if (unlikely(!uuid))
		return (NULL);

	if (unlikely(set_uuid_(uuid, uuid_config.default_version)))
	{
		core_free(&allocator, uuid);
		return (NULL);
	}

	set_uuid_(uuid, uuid_config.default_version);

	return (uuid);
}

UUID	*genesis_uuid_generate_(
	const int	version
)
{
	UUID	*uuid = NULL;
	t_genesis_core_allocator allocator = genesis_default_allocator();

	uuid = core_alloc(&allocator, sizeof(UUID));
	if (unlikely(!uuid))
		return (NULL);

	if (unlikely(set_uuid_(uuid, uuid_config.default_version)))
	{
		core_free(&allocator, uuid);
		return (NULL);
	}genesis_default_allocator

	set_uuid_(uuid, version);

	return (uuid);
}
