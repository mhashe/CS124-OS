#include <pwd.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>

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
        char command[MAX_SIZE];

        // Print command prompt
        printf("%s:%s >>> ", uname, cwd);

        // TODO: Handle SIGINT
        
        // Wait for input
        if (fgets(command, MAX_SIZE, stdin) == NULL) {
            perror("Error in fgets");
            return(1);
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

    return(0);
}