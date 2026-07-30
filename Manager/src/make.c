/**
 * @file make.c
 * @brief Generate the Genesis Make build files for a selected module set.
 */

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include <linux/limits.h>

#include "manager.h"

static int module_has_selected_parent(
	const t_array *const modules,
	const size_t index
)
{
	const t_module *const module = modules->data[index];

	for (size_t i = 0; i < modules->length; ++i)
	{
		const t_module *const candidate = modules->data[i];
		size_t parent_len;

		if (i == index || !candidate || !candidate->path || !module
			|| !module->path)
			continue ;
		parent_len = strlen(candidate->path);
		if (!strncmp(module->path, candidate->path, parent_len)
			&& module->path[parent_len] == '/')
			return (true);
	}
	return (false);
}

static int write_root_makefile(
	const Config *const config,
	const t_array *const modules
)
{
	char path[PATH_MAX];
	FILE *file;

	if (snprintf(path, sizeof(path), "%s/Makefile",
		config->dest ? config->dest : ".") >= (int)sizeof(path))
		return (-ENAMETOOLONG);
	file = fopen(path, "w");
	if (!file)
		return (-errno);

	fprintf(file,
		"NAME            := genesis\n"
		"AR              ?= ar\n"
		"RM              ?= rm -f\n"
		"BUILD_DIR       := build\n"
		"MODULES         :=");
	for (size_t i = 0; i < modules->length; ++i)
	{
		const t_module *const module = modules->data[i];

		if (module && module->path && *module->path
			&& !module_has_selected_parent(modules, i))
			fprintf(file, " %s", module->path);
	}
	fprintf(file,
		"\nTARGET          := $(BUILD_DIR)/lib$(NAME).a\n"
		"\n"
		".DEFAULT_GOAL := all\n"
		"\n"
		".PHONY: all $(MODULES) clean fclean re\n"
		"\n"
		"all: $(TARGET)\n"
		"\n"
		"$(TARGET): $(MODULES)\n"
		"\t@mkdir -p $(dir $@)\n"
		"\t@$(RM) $@\n"
		"\t@find $(BUILD_DIR) -name '*.o' -print0 | xargs -0 -r $(AR) rcs $@\n"
		"\t@test -f $@ || $(AR) rcs $@\n"
		"\n"
		"$(MODULES):\n"
		"\t$(MAKE) -C $@ ROOT_DIR=$(CURDIR) BUILD_DIR=$(CURDIR)/$(BUILD_DIR)\n"
		"\n"
		"clean:\n"
		"\t@for module in $(MODULES); do \\\n"
		"\t\t$(MAKE) -C $$module ROOT_DIR=$(CURDIR) BUILD_DIR=$(CURDIR)/$(BUILD_DIR) clean; \\\n"
		"\tdone\n"
		"\n"
		"fclean: clean\n"
		"\t$(RM) -r $(BUILD_DIR)\n"
		"\n"
		"re: fclean all\n");
	fclose(file);
	return (error_none);
}

int make_write(
	const Config *const config,
	const t_array *const modules
)
{
	char source[PATH_MAX];
	char destination[PATH_MAX];
	int err;

	if (!config || !modules || !config->consts.path_cache_dir)
		return (-EINVAL);
	if (snprintf(source, sizeof(source), "%s/mk",
		config->consts.path_cache_dir) >= (int)sizeof(source)
		|| snprintf(destination, sizeof(destination), "%s/mk",
		config->dest ? config->dest : ".") >= (int)sizeof(destination))
		return (-ENAMETOOLONG);
	err = copy_dir(source, destination);
	if (err)
		return (err);
	return (write_root_makefile(config, modules));
}
