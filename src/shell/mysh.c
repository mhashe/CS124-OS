#include <stdlib.h>
#include <pwd.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <signal.h>

#include "token.h"

#define MAX_SIZE    1024    // Max size of input

int main(int argc, char *argv[])
{
    // User ID. Function guaranteed to be successful.
    uid_t uid = getuid();

    // Struct containing user information
    struct passwd *pw = getpwuid(uid);
    if (!pw)
    {
        perror("Error in getpwuid");
        return 1;
    }
    // User name, home directory
    char *uname = pw->pw_name;
    char *hdir = pw->pw_dir;

    // Path length at most PATH_MAX
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("Error in getcwd");
        return 1;
    }

    // If process in home directory of user, change prompt
    if (strstr(cwd, hdir) != NULL) {
        // Copy to hold cwd
        char copy[PATH_MAX];
        strcpy(copy, cwd); // No return value for error conditions

        // Replace /home/USER with ~
        cwd[0] = '~';
        memmove(&cwd[1], &copy[strlen(hdir)], PATH_MAX - strlen(hdir));
    }

    while(1) {
        char command[MAX_SIZE];

        // Print command prompt
        printf("%s:%s >>> ", uname, cwd);

        // TODO: Handle SIGINT

        // Wait for input
        if (fgets(command, MAX_SIZE, stdin) == NULL) {
            // Received EOF => stdin is closed, no reason for terminal
            printf("\n");
            return 0;
        }

        // Tokenize input
        char** comms = tokenize(command);

        char* comm;
        int i = 0;
        while(1) {
            comm = comms[i];
            if (comm == NULL) {
                break;
            }
            printf("%s\n", comm);
            i++;

        }
    }

    return 0;
}