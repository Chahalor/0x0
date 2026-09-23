// Header

#define _GNU_SOURCE

/* ----| Headers    |----- */
	/* Standard */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

	/* Internal */
#include "_dotenv.h"

	/* External */
		//...

/* ----| Prototypes |----- */
	//...

/* ----| Internals  |----- */

static char *_getline(
	FILE *const _file,
	const char *_end
)
{
	char	_buffer[BUFFER_SIZE];
	size_t	_endlen = strlen(_end);
	size_t	_n;
	long	_start;
	size_t	_offset;
	long	_end_pos = -1;
	long	_after = -1;
	size_t	_size;
	char	*result = NULL;

	_start = ftell(_file);
	if (_start < 0)
		goto error;

	while ((_n = fread(_buffer, 1, BUFFER_SIZE, _file)) > 0)
	{
		void	*p = (_endlen == 1) ?
						memchr(_buffer, _end[0], _n) :
						memmem(_buffer, _n, _end, _endlen);

		if (p)
		{
			_offset = (char *)p - _buffer;
			_end_pos = ftell(_file) - _n + _offset;
			_after = _end_pos + _endlen;
			fseek(_file, _after, SEEK_SET);
			break;
		}
	}

	if (_end_pos < 0)
	{
		if (ferror(_file))
			return NULL;
		_end_pos = ftell(_file);
		_after = _end_pos;
		if (_end_pos == _start && _after == _start)
			return NULL;
	}

	_size = _end_pos - _start;
	result = mem_alloc(_size + 1);
	if (!result)
		goto error;

	fseek(_file, _start, SEEK_SET);
	fread(result, 1, _size, _file);
	result[_size] = '\0';
	if (_after >= 0)
		fseek(_file, _after, SEEK_SET);

error:
	return (result);
}

static int	_is_key_start(const char c)
{
	return (
		(c >= 'a' && c <= 'z')
		|| (c >= 'A' && c <= 'Z')
		|| c == '_'
	);
}

static int	_is_key_char(const char c)
{
	return (
		_is_key_start(c)
		|| (c >= '0' && c <= '9')
	);
}

static char	*_skip_spaces(char *str)
{
	while (*str == ' ' || *str == '\t')
		str++;
	return (str);
}

static void	_trim_end(char *start, char *end)
{
	while (
		end > start
		&& (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\r')
	)
		end--;
	*end = '\0';
}

/**
 * @brief Parse an environment line into a key/value pair.
 *
 * The returned key and value point directly inside `line`.
 * No allocation is performed.
 *
 * Supported syntax:
 *   KEY=value
 *   KEY = value
 *   EMPTY=
 *   # comment
 *   <empty line>
 *
 * @return One of e_interface_dotenv_error.
 */
t_interface_dotenv_error	_parse_line(
	char		*const line,
	t_env_pair	*const output
)
{
	char	*cursor;
	char	*key;
	char	*key_end;
	char	*value;
	char	*value_end;

	if (!line || !output)
		return (dotenv_error_system);

	output->key = NULL;
	output->value = NULL;

	cursor = _skip_spaces(line);

	/*
	 * Empty lines and comments don't contain a variable.
	 * They are valid dotenv input.
	 */
	if (!*cursor || *cursor == '#')
		return (dotenv_error_success);

	/*
	 * Parse key.
	 */
	key = cursor;
	if (!_is_key_start(*cursor))
	{
		if (SETTINGS.ignore_invalid_line)
			return (dotenv_error_success);
		return (dotenv_error_invalid_key);
	}

	cursor++;
	while (_is_key_char(*cursor))
		cursor++;

	key_end = cursor;

	/*
	 * Spaces between the key and '=' are allowed.
	 */
	cursor = _skip_spaces(cursor);

	if (*cursor != '=')
	{
		if (SETTINGS.ignore_invalid_line)
			return (dotenv_error_success);
		return (dotenv_error_syntax);
	}

	/*
	 * Remove trailing whitespace from the key.
	 */
	_trim_end(key, key_end);

	if (!*key)
	{
		if (SETTINGS.ignore_invalid_line)
			return (dotenv_error_success);
		return (dotenv_error_missing_key);
	}

	cursor++;
	value = _skip_spaces(cursor);

	/*
	 * Trim whitespace / CR left by CRLF files.
	 */
	value_end = value + strlen(value);
	_trim_end(value, value_end);

	if (!*value && SETTINGS.disallow_empty_variables)
	{
		if (SETTINGS.ignore_invalid_line)
			return (dotenv_error_success);
		return (dotenv_error_empty_value);
	}

	output->key = key;
	output->value = value;
	return (dotenv_error_success);
}

t_interface_dotenv_error	_export_variable(
	const t_env_pair	*const	pair
)
{
	if (unlikely(!pair))
		return (0);

	fprintf(stderr, "variable_overwrite = %d", SETTINGS.variable_overwrite);	//rm
	const int	r = setenv(pair->key, pair->value, SETTINGS.variable_overwrite);

	return (r ? dotenv_error_system : dotenv_error_success);
}

/* ----| Public     |----- */

t_interface_dotenv_error	_dotenv_load_file(
	const char	*const	_path
)
{
	FILE	*_file;
	char	*_line;
	int		result;

	_file = NULL;
	result = error_none;

	_file = fopen(_path, "r");
	if (unlikely(!_file))
	{
		result = dotenv_error_system;
		goto error;
	}

	while ((_line = _getline(_file, "\n")))
	{
		t_env_pair	pair = {0};

		result = _parse_line(_line, &pair);

		if (unlikely(result))
		{
			mem_free(_line);
			goto error;
		}

		result = _export_variable(&pair);
		if (unlikely(result))
		{
			mem_free(pair.key);
			mem_free(pair.value);
			mem_free(_line);
			goto error;
		}

		mem_free(pair.key);
		mem_free(pair.value);
		mem_free(_line);
	}

error:
	if (_file)
		fclose(_file);
	return (result);
}