// Header

# pragma once

/* ************************************************************************** */
/*                                 Headers                                    */
/* ************************************************************************** */

/* -----| Systems   |----- */
# include <dirent.h>
# include <sys/stat.h>

/* -----| Globals   |----- */
# include "lib_config.h"
# include "lib_standards.h"

/* -----| Internals |----- */
# include "types.h"
#include <fcntl.h>

/* -----| Modules   |----- */
	//...

/* ************************************************************************** */
/*                                 Macros                                     */
/* ************************************************************************** */

#ifndef PATH_MAX
# define PATH_MAX	4096
#endif //!PATH_MAX

/* ************************************************************************** */
/*                                 Prototypes                                 */
/* ************************************************************************** */

// Manipulation

# define	fs_path_init(dest, src)	_Generic((src),		\
	const PATH		*const	path:	fs_path_init_path,	\
	const PATH_VIEW	*const	view:	fs_path_init_view,	\
	const char		*const	string:	fs_path_init_str	\
)(dest, src)

int	fs_path_init_path(
	PATH		*const	dest,
	const PATH	*const	path
);

int	fs_path_init_view(
	PATH			*const	dest,
	const PATH_VIEW	*const	view
);

int	fs_path_init_str(
	PATH		*const	dest,
	const char	*const	string
);

# define	fs_path_join(dest, src)	(_Generic((src),	\
	const PATH	*const:	fs_path_join_path,				\
	const char	*const:	fs_path_join_str,				\
	))(dest, src)

int	fs_path_join_path(
	PATH		*const	dest,
	const PATH	*const	src
);

int	fs_path_join_str(
	PATH		*const	dest,
	const char	*const	src
);

# define	fs_path_split(dest, src, diff)	// TODO: add the generic macro
int	fs_path_split_offset(
	PATH		*const	dest,
	const PATH	*const	src,
	const size_t		offset
);

// split the path from src when the path from diff is not the same
// src:  /home/user/dir/file.txt
// diff: /home/user/other_fir/tkt.md
// dest: dir/file.txt
int	fs_path_split_PATH(
	PATH		*const	dest,
	const PATH	*const	src,
	const PATH	*const	diff
);

// same logic than before but with a char *
int	fs_path_split_str(
	PATH		*const	dest,
	const PATH	*const	src,
	const char	*const	diff
);

# define	fs_dir(path, dest)	// TODO: add the generic macro
void	fs_dir_view(
	const PATH	*const	path,
	PATH_VIEW	*const	dest
);

void	fs_dir_path(
	const PATH	*const	path,
	PATH		*const	dest
);

# define	fs_extension(path, dest)	// TODO: add the generic macro
// return NULL if no extension
// path/file.txt -> "txt"
// path/file     -> NULL
void	fs_extension_str(
	const PATH	*const		path,
	char		*const *	dest
);

void	fs_extension_view(
	const PATH	*const	path,
	PATH_VIEW	*const	view
);

# define	fs_filename(path, dest)	// TODO: add the generic macro
void	fs_filename_str(
	const PATH	*const		path,
	char		*const *	dest
);

void	fs_filename_view(
	const PATH	*const	path,
	PATH_VIEW	*const	dest
);

void	fs_path_normalize(
	PATH	*const	path
);

// set the path.path value to the absolute path
void	fs_path_absolute_set(
	PATH	*const	path
);

# define	fs_path_absolute_get(path, dest)	// TODO: generic macro
void	fs_path_absolute_get_view(
	const PATH	*const	path,
	PATH_VIEW	*const	dest
);

void	fs_path_absolute_get_str(
	const PATH	*const		path,
	char		*const *	dest
);

// set the path.path value to the real path
void	fs_path_real_set(
	PATH	*const	path
);

# define	fs_path_real_get(path, dest)
void	fs_path_real_get_view(
	const PATH	*const	path,
	PATH_VIEW	*const	dest
);

void	fs_path_real_get_str(
	const PATH	*const		path,
	char		*const *	dest
);

# define	fs_path_cmp(p1, p2)
int	fs_path_cmp_2path(
	const PATH	*const	p1,
	const PATH	*const	p2
);

int	fs_path_cmp_2view(
	const PATH_VIEW	*const	v1,
	const PATH_VIEW	*const	v2
);

int	fs_path_cmp_view_path(
	const PATH		*const	path,
	const PATH_VIEW	*const	view
);

int	fs_path_cmp_path_view(
	const PATH_VIEW	*const	view,
	const PATH		*const	path
);

# define	fs_is_absolute(path)	// TODO
int	fs_is_absolute_path(
	const PATH	*const	path
);

int	fs_is_absolute_view(
	const PATH_VIEW	*const	view
);

# define	fs_is_relative(path)	// TODO
int	fs_is_relative_path(
	const PATH	*const	path
);

int	fs_is_relative_view(
	const PATH_VIEW	*const	view
);

// Existance

int	fs_exist(
	const PATH	*path
);

t_genesis_filetype	fs_path_type(
	const PATH	*path
);

t_genesis_filetype	fs_view_type(
	const PATH_VIEW	*path
);

# define	fs_path_is_reg(path)	(fs_path_type(path) == FILETYPE_REGULAR)
# define	fs_path_is_dir(path)	(fs_path_type(path) == FILETYPE_DIRECTORY)
# define	fs_path_is_sym(path)	(fs_path_type(path) == FILETYPE_SYMLINK)
# define	fs_path_is_sock(path)	(fs_path_type(path) == FILETYPE_SOCKET)
# define	fs_path_is_fifo(path)	(fs_path_type(path) == FILETYPE_FIFO)
# define	fs_path_is_bck_d(path)	(fs_path_type(path) == FILETYPE_BLOCK_DEVICE)
# define	fs_path_is_char_d(path)	(fs_path_type(path) == FILETYPE_CHAR_DEVICE)

# define	fs_view_is_reg(path)	(fs_view_type(path) == FILETYPE_REGULAR)
# define	fs_view_is_dir(path)	(fs_view_type(path) == FILETYPE_DIRECTORY)
# define	fs_view_is_sym(path)	(fs_view_type(path) == FILETYPE_SYMLINK)
# define	fs_view_is_sock(path)	(fs_view_type(path) == FILETYPE_SOCKET)
# define	fs_view_is_fifo(path)	(fs_view_type(path) == FILETYPE_FIFO)
# define	fs_view_is_bck_d(path)	(fs_view_type(path) == FILETYPE_BLOCK_DEVICE)
# define	fs_view_is_char_d(path)	(fs_view_type(path) == FILETYPE_CHAR_DEVICE)

# define	fs_is_reg(path)		// TODO -> add the generic macro using the 2 above
# define	fs_is_dir(path)		// TODO -> add the generic macro using the 2 above
# define	fs_is_sym(path)		// TODO -> add the generic macro using the 2 above
# define	fs_is_sock(path)	// TODO -> add the generic macro using the 2 above
# define	fs_is_fifo(path)	// TODO -> add the generic macro using the 2 above
# define	fs_is_bck_d(path)	// TODO -> add the generic macro using the 2 above
# define	fs_is_char_d(path)	// TODO -> add the generic macro using the 2 above
// Metadata

// Creation

int	fs_create_file(
	PATH	*path
);

int	fs_create_tmp_file(
	PATH	*path
);

int	fs_create_dir(
	PATH		*path,
	const int	tree
);

int	fs_create_tmp_dir(
	PATH		*path,
	const int	tree
);

// Deletion

int	fs_delete_file(
	PATH	*path
);

int	fs_delete_dir(
	PATH		*path,
	const int	recursive
);

// delete any thing pointed by the path
int	fs_delete(
	PATH	*path
);

// Copy

int	fs_copy_file(
	PATH	*dest,
	PATH	*src,
	const int	flags
);

int	fs_copy_dir(
	PATH		*dest,
	PATH		*src,
	const int	recursive,
	const int	flags
);

// Move / Rename

int	fs_move_file(
	PATH	*dest,
	PATH	*src
);

int	fs_move_dir(
	PATH		*dest,
	PATH		*src,
	const int	tree
);

int	fs_rename_path(
	PATH	*path,
	char	*name
);

// Read

# define	fs_fopen(path, mode)	// TODO
FILE	*fs_fopen_path(
	PATH		*			path,
	const char	*restrict	mode
);

FILE	*fs_fopen_view(
	PATH_VIEW	*			path,
	const char	*restrict	mode
);

# define	fs_opendir(path)	// TODO
DIR	*fs_dopen_path(
	PATH	*path
);

DIR	fs_dopen_view(
	PATH_VIEW	*path
);

// Links

int	fs_sym_create(
	PATH		*dest,
	const PATH	*src
);

# define fs_symlink_create(target, link)	// TODO
// Create a symbolic link
int	fs_symlink_create_path_path(
	const PATH	*target,
	const PATH	*link
);

// Create a symbolic link
int	fs_symlink_create_path_view(
	const PATH		*target,
	const PATH_VIEW	*link
);

// Create a symbolic link
int	fs_symlink_create_view_path(
	const PATH_VIEW	*target,
	const PATH		*link
);

// Create a symbolic link
int	fs_symlink_create_view_view(
	const PATH_VIEW	*target,
	const PATH_VIEW	*link
);

# define	fs_symlink_read	// TODO
// Read the stored target of a symbolic link
int	fs_symlink_read_path(
	const PATH	*link,
	PATH		*dest
);

int	fs_symlink_read_view(
	const PATH_VIEW	*link,
	PATH			*dest
);

# define	fs_path_real(src, dest)	// TODO
// Resolve the complete path
int fs_path_real_path(
	const PATH	*src,
	PATH		*dest
);

// Resolve the complete path
int fs_path_real_view(
	const PATH	*src,
	PATH		*dest
);

// Remove a symbolic link
# define	fs_symlink_remove(path)	// TODO
int	fs_symlink_remove_path(
	const PATH	*path
);

int	fs_symlink_remove_view(
	const PATH	*path
);

// Perms

# define	fs_chmod(path, mode)	// TODO
int	fs_chmod_path(
	PATH	*path,
	mode_t	mode
);

int	fs_chmod_view(
	PATH_VIEW	*path,
	mode_t		mode
);

# define	fs_chown(path, owner, group)	// TODO
int	fs_chown_path(
	const PATH	*path,
	uid_t		owner,
	git_t		 	
)