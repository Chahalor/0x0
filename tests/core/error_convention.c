#include "core.h"

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int	main(void)
{
	t_genesis_core_buffer	buffer;
	t_genesis_core_settings	settings;
	t_genesis_core_allocator	allocator;
	const char				*message;
	int						status;

	allocator = core_allocator_system();
	settings = core_settings_default();
	assert(core_allocator_validate(&allocator) == 0);
	assert(core_allocator_validate(NULL) == EINVAL);
	assert(core_settings_set_allocator(NULL, &allocator) == EINVAL);
	assert(core_settings_set_allocator(&settings, &allocator) == 0);
	assert(core_buffer_init(NULL, 4) == EINVAL);
	assert(core_buffer_init(&buffer, 4) == 0);
	assert(core_buffer_append(&buffer, "ab", 2) == 0);
	status = core_buffer_append(&buffer, buffer.bytes + 1, 3);
	assert(status == -BUFFER_ERROR_SOURCE_OUT_OF_RANGE);
	assert(core_buffer_append_fixed(&buffer, buffer.bytes + 1, 2)
		== -BUFFER_ERROR_SOURCE_OUT_OF_RANGE);
	assert(buffer.length == 2 && buffer.capacity == 4);
	message = core_buffer_strerror(status);
	assert(strcmp(message, "Source range is outside the buffer contents") == 0);
	assert(strcmp(message, genesis_strerror(GENESIS_MODULE_BUFFER, status)) == 0);
	assert(core_buffer_append(&buffer, buffer.bytes + 1, 1) == 0);
	assert(memcmp(buffer.bytes, "abb", 3) == 0);
	assert(core_buffer_append_fixed(&buffer, "abc", 3) == ENOSPC);
	assert(strcmp(genesis_strerror(GENESIS_MODULE_BUFFER, ENOSPC),
		strerror(ENOSPC)) == 0);
	assert(strcmp(genesis_strerror(GENESIS_MODULE_BUFFER, 0), "Success") == 0);
	assert(strcmp(genesis_strerror(GENESIS_MODULE_ERROR, -CORE_ERROR_UNKNOWN),
		"Unknown core error") == 0);
	assert(strcmp(genesis_strerror(GENESIS_MODULE_ALLOCATOR,
		-ALLOCATOR_ERROR_UNKNOWN), "Unknown allocator error") == 0);
	assert(strcmp(genesis_strerror(GENESIS_MODULE_SETTING,
		-SETTING_ERROR_UNKNOWN), "Unknown setting error") == 0);
	assert(strcmp(genesis_strerror(GENESIS_MODULE_BUFFER, -500),
		"Unknown buffer error") == 0);
	assert(strcmp(genesis_strerror(GENESIS_MODULE_BUFFER, INT_MIN),
		"Unknown buffer error") == 0);
	assert(strcmp(genesis_strerror((t_genesis_core_module)100, EINVAL),
		"Unknown Genesis module") == 0);
	errno = 0;
	assert(core_alloc_array(&allocator, SIZE_MAX, 2) == NULL);
	assert(errno == EOVERFLOW);
	errno = 0;
	assert(core_settings_allocator(NULL) == NULL);
	assert(errno == EINVAL);
	core_buffer_deinit(&buffer);
	return (0);
}
