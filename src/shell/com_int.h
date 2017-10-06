#ifndef COM_INT_H
#define COM_INT_H

/*
 * If cmd is internal, execute it and return 1. Else, return 0 and deal with the
 * fact that it may not be the only_command in the terminal by doing nothing
 * (replicate behavior or actual terminal (ie. exit | echo "hi").
 */
int internal_command_handler(struct command *cmd, int only_command);


#endif