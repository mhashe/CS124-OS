#include <unistd.h> // read, write
#include <stdio.h> // cout, cin
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h> // pid_t, wait
#include <string.h>

#include "com_parser.h"
#include "mysh.h"

// if cmd is internal, execute it and return 1. else, return 0
// deal with the fact that it may not be the only_command in the terminal by doing nothing (replicate behavior or actual terminal (ie. (ie. exit | echo "hi")))
int internal_command_handler(struct command *cmd, int only_command) {
    char *cmd_name = cmd->exec_fn;

    // switch statement in c won't work well
    if (strcmp(cmd_name, "exit") == 0) {
        if (only_command) {
            exit(0);
        }
    } else if ((strcmp(cmd_name, "cd") == 0) || (strcmp(cmd_name, "chdir") == 0)) {
        if ((cmd->argv[1] == NULL) || (strcmp(cmd->argv[1], "~") == 0)) {
            chdir(getenv("HOME"));
        } else {
            chdir(cmd->argv[1]);
        }
    } else if (strcmp(cmd_name, "history") == 0) {
        // TODO: maybe, in the future, allow piping the results of this into the next cmd
        print_history();
    } else {
        return 0;
    }
    return 1;
}
