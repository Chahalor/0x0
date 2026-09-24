#include "core.h"

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

struct s_stats
{
	size_t	allocations;
	size_t	releases;
};

static void	*tracked_alloc(
	void		*const	context,
	const size_t		size
)
{
	struct s_stats	*stats;

	stats = context;
	stats->allocations++;
	return (malloc(size));
}

static void	tracked_free(
	void		*const	context,
	void		*const	ptr,
	const size_t		size
)
{
	struct s_stats	*stats;

	(void)size;
	stats = context;
	stats->releases++;
	free(ptr);
}

int	main(void)
{
	t_genesis_settings			settings;
	t_genesis_settings			other;
	t_genesis_core_allocator	allocator;
	t_genesis_core_allocator	selected;
	t_genesis_core_buffer		stack_buffer;
	t_genesis_core_buffer_settings	growth;
	t_genesis_core_buffer		*buffer;
	t_genesis_core_buffer		*exact_buffer;
	struct s_stats				stats;
	int							value;
	const t_genesis_setting_key	missing = {
		.name = "test.missing", .size = sizeof(int)
	};

	stats = (struct s_stats){0};
	assert(genesis_settings_init(&settings) == 0);
	assert(genesis_settings_init(&other) == 0);
	assert(genesis_settings_get(&settings, &genesis_allocator_setting,
		&selected) == 0);
	assert(core_allocator_validate(&selected) == 0);
	assert(genesis_settings_get(&settings, &missing, &value)
		== -SETTING_ERROR_NOT_CONFIGURED);
	assert(strcmp(genesis_strerror(GENESIS_MODULE_SETTING,
		-SETTING_ERROR_NOT_CONFIGURED), "Setting is not configured") == 0);
	allocator = (t_genesis_core_allocator){
		.context = &stats, .alloc = tracked_alloc,
		.realloc = NULL, .free = tracked_free
	};
	assert(genesis_settings_set(&settings, &genesis_allocator_setting,
		&allocator) == 0);
	assert(genesis_settings_get(&other, &genesis_allocator_setting,
		&selected) == 0);
	assert(selected.context == NULL);
	allocator.alloc = NULL;
	assert(genesis_settings_set(&settings, &genesis_allocator_setting,
		&allocator) == EINVAL);
	assert(genesis_settings_get(&settings, &genesis_allocator_setting,
		&selected) == 0);
	assert(selected.alloc == tracked_alloc);
	value = 7;
	assert(genesis_settings_set(&settings, &missing, &value) == 0);
	value = 0;
	assert(genesis_settings_get(&settings, &missing, &value) == 0);
	assert(value == 7);
	growth = (t_genesis_core_buffer_settings){
		.initial_capacity = 8,
		.growth = BUFFER_GROWTH_GEOMETRIC
	};
	assert(genesis_settings_set(&settings, &genesis_buffer_setting,
		&growth) == 0);
	assert(core_buffer_init_with_settings(&stack_buffer, 0, &settings) == 0);
	assert(core_buffer_append(&stack_buffer, "x", 1) == 0);
	assert(stack_buffer.capacity == 8);
	growth.growth = BUFFER_GROWTH_EXACT;
	assert(genesis_settings_set(&settings, &genesis_buffer_setting,
		&growth) == 0);
	growth.initial_capacity = 0;
	assert(genesis_settings_set(&settings, &genesis_buffer_setting,
		&growth) == EINVAL);
	assert(genesis_settings_get(&settings, &genesis_buffer_setting,
		&growth) == 0);
	assert(growth.initial_capacity == 8
		&& growth.growth == BUFFER_GROWTH_EXACT);
	assert(stack_buffer.settings.growth == BUFFER_GROWTH_GEOMETRIC);
	core_buffer_deinit(&stack_buffer);
	buffer = core_buffer_new_with_settings(4, &settings);
	assert(buffer != NULL);
	assert(core_buffer_append(buffer, "abcd", 4) == 0);
	assert(core_buffer_append(buffer, "efgh", 4) == 0);
	assert(memcmp(buffer->bytes, "abcdefgh", 8) == 0);
	exact_buffer = core_buffer_new_with_settings(0, &settings);
	assert(exact_buffer != NULL);
	assert(core_buffer_append(exact_buffer, "xyz", 3) == 0);
	assert(exact_buffer->capacity == 3);
	core_buffer_free(exact_buffer);
	assert(genesis_settings_reset(&settings, &genesis_buffer_setting) == 0);
	assert(genesis_settings_reset(&settings, &genesis_allocator_setting) == 0);
	core_buffer_free(buffer);
	assert(genesis_settings_get(&settings, &genesis_allocator_setting,
		&selected) == 0);
	assert(selected.context == NULL);
	genesis_settings_deinit(&settings);
	genesis_settings_deinit(&other);
	assert(stats.allocations == stats.releases);
	return (0);
}
