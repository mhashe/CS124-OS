#include <pwd.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>

#include "token.h"

#define MAX_SIZE	1024	// Max size of input

int main(int argc, char *argv[])
{
	// User ID. Function guaranteed to be successful.
	uid_t uid = getuid();

	// Struct containing user information
	struct passwd *pw = getpwuid(uid);
	if (!pw)
	{
		perror("Error in getpwuid");
		return(1);
	}
	// User name
	char *uname = pw->pw_name;

	// Path length at most PATH_MAX
	char cwd[PATH_MAX];
	if (getcwd(cwd, sizeof(cwd)) == NULL) {
		perror("Error in getcwd");
		return(1);
	}

	while(1) {
		char commands[MAX_SIZE];

		// Print command prompt
		printf("%s:%s >>> ", uname, cwd);

		// Wait for input
		fgets(commands, MAX_SIZE, stdin);


	}

	return(0);
}