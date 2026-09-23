/**
 * 
*/
/* ----| Headers    |----- */
	/* Standard */
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <limits.h>
#include <fcntl.h>

#include <linux/limits.h>

#include <sys/stat.h>

	/* Internal */
#include "manager.h"

	/* External */
		//...

/* ----| Prototypes |----- */
	//...

/* ----| Internals  |----- */

#define GOTO_ERROR() \
	do \
	{ \
		result = -errno; \
		goto error; \
	} while (0)

static int	copy_file(
	const char *const	src,
	const char *const	dst
)
{
	int			in, out, err;
	ssize_t		n, w;
	char		buf[8192];
	char		*p;
	struct stat	st;

	if (unlikely(stat(src, &st) < 0))
		return (-errno);

	in = open(src, O_RDONLY);
	if (unlikely(in < 0))
		return (-errno);

	out = open(dst, O_WRONLY | O_CREAT | O_TRUNC, st.st_mode & 0777);
	if (unlikely(out < 0))
	{
		err = errno;
		close(in);
		return (-err);
	}

	while (true)
	{
		do
			n = read(in, buf, sizeof(buf));
		while (n < 0 && errno == EINTR);

		if (n <= 0)
			break;

		p = buf;

		while (n > 0)
		{
			do
				w = write(out, p, n);
			while (w < 0 && errno == EINTR);

			if (unlikely(w < 0))
			{
				err = errno;
				close(in);
				close(out);
				return (-err);
			}

			p += w;
			n -= w;
		}
	}

	if (n < 0)
	{
		err = errno;
		close(in);
		close(out);
		return (-err);
	}

	close(in);
	close(out);
	return (0);
}

static char	*fs_str_dup(
	const char *const	src
)
{
	if (unlikely(!src))
		return (NULL);

	return (mem_dup(src, strlen(src) + 1));
}

static int	mkdir_checked(
	const char *const	path
)
{
	struct stat	st;

	if (!mkdir(path, 0700))
		return (error_none);

	if (errno != EEXIST)
		return (-errno);

	if (unlikely(stat(path, &st) || !S_ISDIR(st.st_mode)))
		return (-ENOTDIR);

	return (error_none);
}

static int	mkdir_recursive(
	const char *const	path
)
{
	char	*tmp;
	int		err;

	if (unlikely(!path || !*path))
		return (error_none);

	tmp = fs_str_dup(path);
	if (unlikely(!tmp))
		return (error_alloc_fail);

	for (size_t i = tmp[0] == '/'; tmp[i]; ++i)
	{
		if (tmp[i] != '/')
			continue ;

		tmp[i] = '\0';
		err = mkdir_checked(tmp);
		tmp[i] = '/';
		if (unlikely(err))
		{
			mem_free(tmp);
			return (err);
		}
	}

	err = mkdir_checked(tmp);
	mem_free(tmp);
	return (err);
}

static int	mkdir_parent_mode(
	const char *const	path,
	const mode_t		mode
)
{
	char	parent[PATH_MAX];
	char	*slash;

	if (unlikely(!path || snprintf(parent, sizeof(parent), "%s", path) >= (int)sizeof(parent)))
		return (-ENAMETOOLONG);

	slash = strrchr(parent, '/');
	if (!slash)
		return (error_none);

	*slash = '\0';
	if (!*parent)
		return (error_none);

	return (mkdir_p(parent, mode));
}

static int	copy_optional_file(
	const Config *const	config,
	const char *const	relative_path
)
{
	char	src[PATH_MAX];
	char	dest[PATH_MAX];
	int		err;

	if (!relative_path || !*relative_path)
		return (error_none);

	snprintf(src, sizeof(src), "%s/%s", config->consts.path_cache_dir, relative_path);
	snprintf(dest, sizeof(dest), "%s/%s", config->dest, relative_path);

	err = mkdir_parent_mode(dest, 0777);
	if (unlikely(err < 0))
		return (err);

	return (copy_file(src, dest));
}

static int	write_genesis_header(
	const Config *const	config,
	const t_array *const	modules
)
{
	char	path[PATH_MAX];
	FILE	*file;

	if (unlikely(snprintf(path, sizeof(path), "%s/genesis.h", config->dest) >= (int)sizeof(path)))
		return (-ENAMETOOLONG);

	file = fopen(path, "w");
	if (unlikely(!file))
		return (-errno);

	fprintf(file,
		"#ifndef GENESIS_H\n"
		"# define GENESIS_H\n"
		"\n"
		"# pragma once\n"
		"\n"
	);

	for (size_t i = 0; i < modules->length; i++)
	{
		t_module *const	module = modules->data[i];

		if (module->public_header && *module->public_header)
			fprintf(file, "# include \"%s\"\n", module->public_header);
	}

	fprintf(file,
		"\n"
		"#endif\t// GENESIS_H\n"
	);
	fclose(file);
	return (error_none);
}

/* ----| Public     |----- */

int dir_rm(
	const char *const restrict path
)
{
	DIR				*dir;
	struct dirent	*entry;
	struct stat		st;
	char			fullpath[PATH_MAX];
	int				err = 0;

	if (rmdir(path) == 0)
		return (0);

	if (unlikely(errno != ENOTEMPTY && errno != EEXIST))
	{
		fprintf(stderr, "cannot remove dir %s: %s\n", path, strerror(errno));
		return (-errno);
	}

	dir = opendir(path);
	if (unlikely(!dir))
	{
		fprintf(stderr, "cannot open dir %s: %s\n", path, strerror(errno));
		return (-errno);
	}

	while ((entry = readdir(dir)))
	{
		if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
			continue;

		snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

		if (unlikely(lstat(fullpath, &st)))
		{
			fprintf(stderr, "stat failed on %s: %s\n", fullpath, strerror(errno)); 
			closedir(dir);
			return (-errno);
		}

		if (S_ISDIR(st.st_mode))
			err = dir_rm(fullpath);
		else
			err = unlink(fullpath);

		if (unlikely(err))
		{
			fprintf(stderr, "cannot remove %s: %s\n", fullpath, strerror(errno));
			closedir(dir);
			return (-errno);
		}
	}

	closedir(dir);

	if (unlikely(rmdir(path)))
	{
		fprintf(stderr, "cannot remove dir %s: %s\n", path, strerror(errno));
		return (-errno);
	}

	return (0);
}

char	*path_join(
	const char *const	left,
	const char *const	right
)
{
	char	*result;
	size_t	left_len;
	size_t	right_len;
	size_t	need_slash;

	if (unlikely(!left || !*left || !right || !*right))
		return (NULL);

	left_len = strlen(left);
	right_len = strlen(right);
	need_slash = left[left_len - 1] != '/';
	result = mem_alloc(left_len + need_slash + right_len + 1);
	if (unlikely(!result))
		return (NULL);

	memcpy(result, left, left_len);
	if (need_slash)
		result[left_len++] = '/';

	memcpy(result + left_len, right, right_len + 1);
	return (result);
}

int	mkdir_parent(
	const char *const	path
)
{
	char	*parent;
	char	*slash;
	int		err;

	if (unlikely(!path || !*path))
		return (-EINVAL);

	parent = fs_str_dup(path);
	if (unlikely(!parent))
		return (error_alloc_fail);

	slash = strrchr(parent, '/');
	if (!slash)
	{
		mem_free(parent);
		return (error_none);
	}

	*slash = '\0';
	err = mkdir_recursive(parent);
	mem_free(parent);
	return (err);
}

int	is_dir_empty(
	const char *const	path
)
{
	DIR				*dir;
	struct dirent	*entry;

	dir = opendir(path);
	if (unlikely(!dir))
		return (-errno);

	while ((entry = readdir(dir)))
	{
		if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
			continue ;

		closedir(dir);
		return (0);
	}

	closedir(dir);
	return (1);
}

int	cache_has_git_metadata(
	const char *const	path
)
{
	struct stat	st;
	char		*git_path;
	int			result;

	git_path = path_join(path, ".git");
	if (unlikely(!git_path))
		return (0);

	result = !stat(git_path, &st);
	mem_free(git_path);
	return (result);
}

int mkdir_p(
	const char *	path,
	mode_t			mode
)
{
	char	tmp[PATH_MAX];
	char	*p;
	int		nb_creation = 0;

	if (unlikely(strlen(path) >= sizeof(tmp)))
		return (-ENAMETOOLONG);

	strcpy(tmp, path);

	for (p = tmp + 1; *p; p++)
	{
		if (*p == '/')
		{
			*p = '\0';

			if (unlikely(mkdir(tmp, mode) < 0 && errno != EEXIST))
				return (-errno);

			nb_creation++;
			*p = '/';
		}
	}

	if (unlikely(mkdir(tmp, mode) < 0 && errno != EEXIST))
		return (-errno);

	return (nb_creation++);
}

int	copy_dir(
	const char *const	src,
	const char *const	dst
)
{
	int				result;
	DIR				*dir;
	struct stat		st;
	struct dirent	*entry;
	char			src_path[PATH_MAX];
	char			dst_path[PATH_MAX];

	result = error_none;

	dir = opendir(src);
	if (unlikely(!dir))
		return (-errno);

	if (unlikely(lstat(src, &st) < 0))
		GOTO_ERROR();

	result = mkdir_p(dst, st.st_mode & 0777);
	if (unlikely(result < 0))
		GOTO_ERROR();

	while ((entry = readdir(dir)) != NULL)
	{
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			continue;

		if (snprintf(src_path, sizeof(src_path), "%s/%s", src, entry->d_name) >= (int)sizeof(src_path))
		{
			result = -ENAMETOOLONG;
			goto error;
		}

		if (snprintf(dst_path, sizeof(dst_path), "%s/%s", dst, entry->d_name ) >= (int)sizeof(dst_path))
		{
			result = -ENAMETOOLONG;
			goto error;
		}

		if (unlikely(lstat(src_path, &st) < 0))
			GOTO_ERROR();

		if (S_ISDIR(st.st_mode))
		{
			result = copy_dir(src_path, dst_path);

			if (unlikely(result < 0))
				goto error;
		}
		else if (S_ISREG(st.st_mode))
		{
			result = copy_file(src_path, dst_path);

			if (unlikely(result < 0))
				goto error;
		}
		else if (S_ISLNK(st.st_mode))
		{
			char	link_target[PATH_MAX];
			ssize_t	len;

			len = readlink(src_path, link_target, sizeof(link_target) - 1 );

			if (unlikely(len < 0))
				GOTO_ERROR();

			link_target[len] = '\0';

			if (unlikely(symlink(link_target, dst_path) < 0))
			{
				if (errno != EEXIST)
					GOTO_ERROR();
			}
		}
	}

error:
	closedir(dir);
	return (result);
}

static int	validate_modules(
	const Config *const	config,
	const t_array *const	modules
)
{
	char		path[PATH_MAX];
	struct stat	st;

	for (size_t i = 0; i < modules->length; ++i)
	{
		const t_module *const	module = modules->data[i];

		if (unlikely(!module || !module->path || !*module->path))
			return (-EINVAL);
		if (snprintf(path, sizeof(path), "%s/%s",
			config->consts.path_cache_dir, module->path) >= (int)sizeof(path))
			return (-ENAMETOOLONG);
		if (stat(path, &st) < 0)
		{
			fprintf(stderr, "module source is missing: %s\n", path);
			return (-errno);
		}
		if (unlikely(!S_ISDIR(st.st_mode)))
			return (-ENOTDIR);
		if (!module->public_header || !*module->public_header)
			continue ;
		if (snprintf(path, sizeof(path), "%s/%s",
			config->consts.path_cache_dir, module->public_header)
			>= (int)sizeof(path))
			return (-ENAMETOOLONG);
		if (stat(path, &st) < 0)
		{
			fprintf(stderr, "module public header is missing: %s\n", path);
			return (-errno);
		}
		if (unlikely(!S_ISREG(st.st_mode)))
			return (-EINVAL);
	}
	return (error_none);
}

static int	directory_is_module(
	const char *const	path
)
{
	char		makefile[PATH_MAX];
	struct stat	st;

	if (snprintf(makefile, sizeof(makefile), "%s/makefile", path)
		< (int)sizeof(makefile) && !stat(makefile, &st) && S_ISREG(st.st_mode))
		return (true);
	if (snprintf(makefile, sizeof(makefile), "%s/Makefile", path)
		< (int)sizeof(makefile) && !stat(makefile, &st) && S_ISREG(st.st_mode))
		return (true);
	return (false);
}

static int	copy_module_tree(
	const char *const	src,
	const char *const	dst,
	const int		is_root
)
{
	DIR			*dir;
	struct dirent	*entry;
	struct stat	st;
	char			src_path[PATH_MAX];
	char			dst_path[PATH_MAX];
	int			err;

	dir = opendir(src);
	if (!dir)
		return (-errno);
	if (lstat(src, &st) < 0)
	{
		err = -errno;
		closedir(dir);
		return (err);
	}
	err = mkdir_p(dst, st.st_mode & 0777);
	if (err < 0)
	{
		closedir(dir);
		return (err);
	}
	while ((entry = readdir(dir)))
	{
		if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
			continue ;
		if (snprintf(src_path, sizeof(src_path), "%s/%s", src, entry->d_name)
			>= (int)sizeof(src_path)
			|| snprintf(dst_path, sizeof(dst_path), "%s/%s", dst, entry->d_name)
			>= (int)sizeof(dst_path))
		{
			closedir(dir);
			return (-ENAMETOOLONG);
		}
		if (lstat(src_path, &st) < 0)
		{
			err = -errno;
			closedir(dir);
			return (err);
		}
		if (S_ISDIR(st.st_mode))
		{
			if (is_root && directory_is_module(src_path))
				continue ;
			err = copy_module_tree(src_path, dst_path, false);
		}
		else if (S_ISREG(st.st_mode))
			err = copy_file(src_path, dst_path);
		else if (S_ISLNK(st.st_mode))
		{
			char link_target[PATH_MAX];
			ssize_t len;

			len = readlink(src_path, link_target, sizeof(link_target) - 1);
			if (len < 0)
				err = -errno;
			else
			{
				link_target[len] = '\0';
				err = symlink(link_target, dst_path);
				if (err < 0 && errno == EEXIST)
					err = error_none;
				else if (err < 0)
					err = -errno;
			}
		}
		else
			err = error_none;
		if (err < 0)
		{
			closedir(dir);
			return (err);
		}
	}
	closedir(dir);
	return (error_none);
}

int	copy_modules(
	const Config *const	config,
	const t_array *const	modules
)
{
	char	src[PATH_MAX];
	char	dest[PATH_MAX];
	struct stat	st;
	int		err = 0;

	err = validate_modules(config, modules);
	if (unlikely(err))
		return (err);

	for (size_t i = 0; i < modules->length; i++)
	{
		t_module *const	module = modules->data[i];

		snprintf(src, sizeof(src), "%s/%s", config->consts.path_cache_dir, module->path);
		snprintf(dest, sizeof(dest), "%s/%s", config->dest, module->path);

		if (!stat(src, &st) && S_ISDIR(st.st_mode))
		{
			err = copy_module_tree(src, dest, true);
			if (unlikely(err))
			{
				perror("copy dir");
				return (err);
			}
		}
		else if (errno != ENOENT)
			return (-errno);

		err = copy_optional_file(config, module->public_header);
		if (unlikely(err))
		{
			perror("copy file");
			return (err);
		}
	}

	return (write_genesis_header(config, modules));
}

int	count_dir_files(
	const char *const	path,
	const bool			recursive
)
{
	int				result = 0;
	DIR				*dir;
	struct dirent	*entry;
	struct stat		st;
	char			_path[PATH_MAX];

	dir = opendir(path);
	if (unlikely(!dir))
		return (-errno);

	while ((entry = readdir(dir)) != NULL)
	{
		if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
			continue;

		else if (snprintf(_path, sizeof(_path), "%s/%s", path, entry->d_name) >= (int)sizeof(_path))
			continue;

		else if (lstat(_path, &st) < 0)
			continue;

		else if (S_ISDIR(st.st_mode) && recursive)
		{
			const int	tmp = count_dir_files(_path, true);

			if (unlikely(tmp < 0))
				continue;

			result += tmp;
		}
		else if (S_ISREG(st.st_mode))
				result++;
	}

	closedir(dir);
	return (result);
}

size_t	dir_size(
	const char *const	path
)
{
	size_t			result = 0;
	DIR				*dir;
	struct dirent	*entry;
	struct stat		st;
	char			_path[PATH_MAX];

	dir = opendir(path);
	if (unlikely(!dir))
	{
		fprintf(stderr, "could not open dir %s: %s\n", path, strerror(errno));	//rm
		return (0);
	}

	while ((entry = readdir(dir)) != NULL)
	{
		if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
			continue;

		else if (snprintf(_path, sizeof(_path), "%s/%s", path, entry->d_name) >= (int)sizeof(_path))
			continue;

		else if (lstat(_path, &st) < 0)
			continue;

		else if (S_ISDIR(st.st_mode))
		{
			const int	tmp = dir_size(_path);

			result += tmp;
		}
		else if (S_ISREG(st.st_mode))
				result += st.st_size;
	}

	closedir(dir);
	return (result);
}