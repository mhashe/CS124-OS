#include "com_parser.h"

/* 
 * Takes in a linked list of commands and executes them while redirecting their
 * io. It then waits for all of them to finish in the shell process.
 *
 * Inputs:
 *      cmd: a command struct
 *
 * Returns:
 *      0: fork and execution was successful
 *      1: fork and execution attained an unreachable or unknown state
 *      
*/
int fork_and_exec_commands(struct command *cmd);


/* 
 * Waits for all children processes in the shell's group id to terminate and
 * return. If any terminate with an unexpected status code, log it to the user.
 */
void wait_for_children();


/* 
 * Executes the command after forking and does the io redirection to stdout,
 * stdin, or stderr if needed. The io redirection is done after forking, so the
 * redirection is unique to this process/command. Note: creat() is equivalent to
 * open(path, O_WRONLY|O_CREAT|O_TRUNC, mode) and mode 0644 means write
 * permissions are given to only the owner. If files don't exist, the shell
 * exits, since this kind of error should be caught while parsing, not at
 * execution.
 *
 * Inputs:
 *      cmd: a command struct
 *
*/
void execute_command(struct command *cmd);
