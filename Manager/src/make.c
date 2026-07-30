/**
 * @file make.c
 * @brief Generate a self-contained Make architecture for a 0x0 export.
 */

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include <linux/limits.h>

#include "manager.h"

static int module_path_is_parent(
	const t_module *const parent,
	const t_module *const child
)
{
	size_t parent_len;

	if (!parent || !parent->path || !child || !child->path)
		return (false);
	parent_len = strlen(parent->path);
	return (!strncmp(child->path, parent->path, parent_len)
		&& child->path[parent_len] == '/');
}

static int module_has_selected_parent(
	const t_array *const modules,
	const size_t index
)
{
	const t_module *const module = modules->data[index];

	for (size_t i = 0; i < modules->length; ++i)
	{
		if (i != index && module_path_is_parent(modules->data[i], module))
			return (true);
	}
	return (false);
}

static int module_is_immediate_child(
	const t_module *const parent,
	const t_module *const child
)
{
	const char *relative;

	if (!module_path_is_parent(parent, child))
		return (false);
	relative = child->path + strlen(parent->path) + 1;
	return (*relative && !strchr(relative, '/'));
}

static int module_has_selected_child(
	const t_array *const modules,
	const t_module *const module
)
{
	for (size_t i = 0; i < modules->length; ++i)
	{
		if (module_is_immediate_child(module, modules->data[i]))
			return (true);
	}
	return (false);
}

static void write_root_reference(
	FILE *const file,
	const char *const module_path
)
{
	fputs("..", file);
	for (const char *cursor = module_path; *cursor; ++cursor)
	{
		if (*cursor == '/')
			fputs("/..", file);
	}
}

static int write_module_makefile(
	const Config *const config,
	const t_array *const modules,
	const t_module *const module
)
{
	char path[PATH_MAX];
	FILE *file;
	int is_main;

	if (snprintf(path, sizeof(path), "%s/%s/makefile",
		config->dest ? config->dest : ".", module->path) >= (int)sizeof(path))
		return (-ENAMETOOLONG);
	file = fopen(path, "w");
	if (!file)
		return (-errno);

	fputs("ROOT_DIR        ?= ", file);
	write_root_reference(file, module->path);
	fputc('\n', file);
	is_main = !strchr(module->path, '/')
		|| module_has_selected_child(modules, module);
	if (is_main)
	{
		fputs("SUBMODULES      :=", file);
		for (size_t i = 0; i < modules->length; ++i)
		{
			const t_module *const child = modules->data[i];
			const char *name;

			if (!module_is_immediate_child(module, child))
				continue ;
			name = strrchr(child->path, '/');
			fprintf(file, " %s", name ? name + 1 : child->path);
		}
		fputs("\n\ninclude $(ROOT_DIR)/mk/main-module.mk\n", file);
	}
	else
		fputs("\ninclude $(ROOT_DIR)/mk/sub-module.mk\n", file);
	fclose(file);
	return (error_none);
}

static int write_module_makefiles(
	const Config *const config,
	const t_array *const modules
)
{
	int err;

	for (size_t i = 0; i < modules->length; ++i)
	{
		err = write_module_makefile(config, modules, modules->data[i]);
		if (err)
			return (err);
	}
	return (error_none);
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

static void write_manifest_array(
	FILE *const file,
	const char *const name,
	const t_array *const values
)
{
	fprintf(file, "%s = [\n", name);
	for (size_t i = 0; i < values->length; ++i)
	{
		const TOML *const node = values->data[i];

		if (node && node->data)
			fprintf(file, "\t\"%s\",\n", node->data);
	}
	fputs("]\n", file);
}

static int write_manifest(
	const Config *const config,
	const t_array *const modules
)
{
	char path[PATH_MAX];
	FILE *file;

	if (snprintf(path, sizeof(path), "%s/GENESIS.toml",
		config->dest ? config->dest : ".") >= (int)sizeof(path))
		return (-ENAMETOOLONG);
	file = fopen(path, "w");
	if (!file)
		return (-errno);
	fprintf(file,
		"name = \"%s\"\n"
		"version = \"%s\"\n"
		"url = \"%s\"\n"
		"\n"
		"[export]\n"
		"includeDir = \".\"\n"
		"sourceDir = \".\"\n"
		"buildSystem = \"make\"\n",
		config->lib.name ? config->lib.name : "0x0 - Genesis",
		config->lib.version ? config->lib.version : VERSION,
		config->lib.remote_url ? config->lib.remote_url : DEFAULT_URL_GIT);
	for (size_t i = 0; i < modules->length; ++i)
	{
		const t_module *const module = modules->data[i];

		fprintf(file,
			"\n[%s]\n"
			"path = \"%s\"\n"
			"publicHeaders = \"%s\"\n"
			"privateHeaders = \"%s\"\n",
			module->name, module->path,
			module->public_header ? module->public_header : "",
			module->private_header ? module->private_header : "");
		write_manifest_array(file, "dependencies", &module->dependencies);
		write_manifest_array(file, "defines", &module->defines);
		write_manifest_array(file, "tags", &module->tags);
		write_manifest_array(file, "controls", &module->controls);
	}
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
	err = write_module_makefiles(config, modules);
	if (err)
		return (err);
	err = write_root_makefile(config, modules);
	if (err)
		return (err);
	return (write_manifest(config, modules));
}
