#ifndef GENESIS_INTERFACE_DOTENV_SETTINGS_H
# define GENESIS_INTERFACE_DOTENV_SETTINGS_H

# include <stdio.h>

typedef struct s_interface_dotenv_settings
{
	char	variable_overwrite;	// default beavore when loading the .env file
	char	ignore_invalid_line;
	char	disallow_empty_variables;
	char	print_error;
	FILE	*output;
	char	**default_env_paths;	// NULL terminated array of C-strings
}	t_interface_dotenv_settings;


extern t_interface_dotenv_settings	g_dotenv_settings;

#endif //!GENESIS_INTERFACE_DOTENV_SETTINGS_H