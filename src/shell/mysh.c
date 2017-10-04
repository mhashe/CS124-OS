#include <pwd.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>

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

	printf("%s\n", uname);
	printf("%s\n", cwd);
}