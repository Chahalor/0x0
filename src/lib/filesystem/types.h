// Header

# pragma once

/* ************************************************************************** */
/*                                 Headers                                    */
/* ************************************************************************** */

/* -----| Systems   |----- */
# include <limits.h>
# include <time.h>
# include <sys/stat.h>
# include <stdint.h>

/* -----| Globals   |----- */
	//...

/* -----| Internals |----- */
# include "filesystem.h"

/* -----| Modules   |----- */
# include "core/buffer/buffer.h"

/* ************************************************************************** */
/*                                 Typedefs                                   */
/* ************************************************************************** */
/* -----| Typename  |----- */
	//...

/* -----| Enums     |----- */
	//...

/* -----| Unions    |----- */
	//...

/* -----| Structs   |----- */
	//...

/* ************************************************************************** */
/*                                 Enums                                      */
/* ************************************************************************** */

typedef enum e_genesis_filetype
{
	FILETYPE_UNKNOWN,
	FILETYPE_REGULAR,
	FILETYPE_DIRECTORY,
	FILETYPE_SYMLINK,
	FILETYPE_SOCKET,
	FILETYPE_FIFO,
	FILETYPE_BLOCK_DEVICE,
	FILETYPE_CHAR_DEVICE,
}	t_genesis_filetype;

typedef enum e_genesis_path_error
{
	PATH_SUCCESS,
	PATH_ERROR_INVALID,
	PATH_ERROR_TOO_LONG,
	PATH_ERROR_NOT_FOUND,
	PATH_ERROR_ACCESS,
	PATH_ERROR_SYSTEM,
	PATH_ERROR_UNKNOW
} t_genesis_path_error;

enum e_genesis_fs_copy_instr
{
	FS_COPY_INSTR_OVERWRITE		= 1 << 0,
	FS_COPY_INSTR_PRESEV_PERMS	= 1 << 1,
	FS_COPY_INSTR_PRESEV_OWNER	= 1 << 2,
	FS_COPY_INSTR_TIMESTP		= 1 << 4,
	FS_COPY_INSTR_FLW_SYM		= 1 << 8
}

/* ************************************************************************** */
/*                                 Unions                                     */
/* ************************************************************************** */
	//...

/* ************************************************************************** */
/*                                 Structs                                    */
/* ************************************************************************** */

typedef struct s_genesis_file_hash
{
	uint64_t	value;
	time_t		computed_at;
} t_genesis_file_hash;

typedef struct s_genesis_path
{
	t_genesis_core_buffer	*path;
	size_t					name_offset;		// ptr from path where the filename begin
	size_t					ext_offset;			// ptr from path where the last extensions begin (after the last '.')
	t_genesis_filetype		types;
	int						flags;				// more data (exist, is_dir)
}	PATH;

typedef struct s_genesis_path_view
{
	const PATH	*src;
	size_t		offset;
	size_t		length;
}	PATH_VIEW;