#include <stdio.h>
 #include <stdlib.h>
#include "../../dotenv.h"
#include "../settings.h"

extern char **environ;

int main(int argc, char const *argv[])
{
	dotenv_load();
	g_dotenv_settings.variable_overwrite = 1;

	for (size_t i = 0; environ[i]; i++)
	{
		printf("%ld: %s\n", i, environ[i]);
	}
	
	return 0;
}
