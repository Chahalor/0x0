// Header

/* ----| Headers    |----- */
	/* Standard */
		//...

	/* Internal */
#include "_internal_/_dotenv.h"

	/* External */
		//...

/* ----| Prototypes |----- */
	//...

/* ----| Internals  |----- */
	//...

/* ----| Public     |----- */

static char	*g_default_env_paths[] = {
	".env",
	NULL
};

t_interface_dotenv_settings	g_dotenv_settings = {
	.variable_overwrite = 0,
	.ignore_invalid_line = 0,
	.disallow_empty_variables = 0,
	.print_error = 0,
	.output = NULL,
	.default_env_paths = g_default_env_paths
};

/**
 * @brief	load the file .env into the programm env
 */
int	dotenv_load(void)
{
	int	err = 0;

	for (
		size_t	i = 0;
		SETTINGS.default_env_paths[i] && !err;
		i++
	)
	{
		err = _dotenv_load_file(SETTINGS.default_env_paths[i]);
	}
	
	return (err);
}

/**
 * @brief	load the file at `path` into the env
 */
int	dotenv_load_file(
	const char	*const	_path
)
{
	return (_dotenv_load_file(_path));
}
