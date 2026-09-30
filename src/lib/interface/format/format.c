/**
 * @file format.c
 * @brief Public, validated entry points for in-memory formatting.
 */

/* ----| Headers    |----- */
	/* Standard */
# include <errno.h>
# include <stdint.h>

	/* Internal */
# include "../format.h"
# include "_internal_/_format.h"

/* ----| Public     |----- */
char	*genesis_vformat(
	const char	*format,
	va_list		args
)
{
	if (!format)
	{
		errno = EINVAL;
		return (NULL);
	}

	return (_format_vformat(format, args));
}

char	*genesis_format(
	const char	*format,
	...
)
{
	va_list	args;
	char	*result;

	if (!format)
	{
		errno = EINVAL;
		return (NULL);
	}

	va_start(args, format);
	result = _format_vformat(format, args);
	va_end(args);
	return (result);
}

size_t	genesis_vsformat(
	char		*restrict output,
	size_t		size,
	const char	*restrict format,
	va_list		args
)
{
	if (!format || (size != 0 && !output))
	{
		if (size != 0 && output)
			output[0] = '\0';
		errno = EINVAL;
		return (SIZE_MAX);
	}

	return (_format_vsformat(output, size, format, args));
}

size_t	genesis_sformat(
	char		*restrict output,
	size_t		size,
	const char	*restrict format,
	...
)
{
	va_list	args;
	size_t	result;

	if (!format || (size != 0 && !output))
	{
		if (size != 0 && output)
			output[0] = '\0';
		errno = EINVAL;
		return (SIZE_MAX);
	}

	va_start(args, format);
	result = _format_vsformat(output, size, format, args);
	va_end(args);
	return (result);
}

int	genesis_format_set(
	const char		*tag,
	t_format_callback	callback
)
{
	if (!tag || !*tag || !callback)
		return (EINVAL);

	return (_format_set(tag, callback));
}

int	genesis_format_rm(
	const char	*tag
)
{
	if (!tag || !*tag)
		return (EINVAL);

	return (format_registry_remove(tag));
}
