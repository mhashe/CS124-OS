#ifndef COM_INT_H
#define COM_INT_H

/*
 * If cmd the command is internal, it is excuted with the knowledge of 
 * whether it is the only command (can choose to ignore if not). The reason  
 * to do ignore an internal command in a sequence arises from replicating the 
 * behavior of a linux terminal (ie. exit | echo "hi").
 *
 * Inputs:
 *      cmd: a command struct
 *      only_command: 1 if this command is neither preceded nor followed by 
 *          another command, else 0
 *
 * Returns:
 *      1: if command was recognized as internal internal
 *      0: not internal
 *
 */
int internal_command_handler(struct command *cmd, int only_command);


#endif
