#include <unistd.h> // read, write
#include <stdio.h> // cout, cin
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h> // pid_t, wait

#include "com_parser.h"

// if cmd is internal, execute it and return 1. else, return 0
// deal with the fact that it may not be the only_command in the terminal by doing nothing (replicate behavior or actual terminal (ie. (ie. exit | echo "hi")))
int internal_command_handler(struct command *cmd, int only_command) {
    printf("im in internal command handler! %d\n", only_command);
    if (only_command) {

    } else {
        // do nothing to replicate command_handler_behavior
    }
    return 0;
}
