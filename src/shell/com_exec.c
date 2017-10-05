#include <unistd.h> // read, write
#include <stdio.h> // cout, cin
#include <stdlib.h>
#include <assert.h>
#include <fcntl.h> //open, creat
#include <sys/types.h> // wait
#include <sys/wait.h> // wait

#include "consts.h"
#include "com_parser.h"
#include "com_exec.h"

/* 
 * Handles forking off and executing a linked list of command that are piped 
 * together and executed in parrallel. It setups up redirection and pipes in 
 * the child processes (commands). It then waits for the processes to complete 
 * in the shell process.
 */

// TODO: Do more advanced testing
// TODO: Improve commenting
// TODO: Either 1. Remove debug info or 2. Make debug mode
// TODO: Error handling

/* Takes in a linked list of commands and executes them while redirecting their 
io. It then waits for all of them to finish in the shell process. */
int fork_and_exec_commands(struct command *cmd) {
    int last_out_fd = -1; // there was no previous fd to take as input

    // Having spawning processes in an infinite while loop seems dangerous: change?
    while (1) {
        pid_t pid;
        int fd[2];

        printf("Handling: %s\n", cmd->exec_fn);

        if (cmd->next != NULL) {
            printf("making pipe!\n");
            if (pipe(fd) < 0)
                return -1; // pipe error
        }
        if ((pid = fork()) < 0) {
            return -1; // fork error
        }
        if (pid > 0) {
            // we are in the shell process
            close(fd[1]); // uncomment out
            if (last_out_fd != -1) {
                printf("%s: closing last_out_fd\n", cmd->exec_fn);
                close(last_out_fd);
            } 
            if (cmd->next == NULL) {
                break; // we are done forking all commands
            } else {
                printf("%s: setting last_out_fd from fd[0], and going next\n", cmd->exec_fn);
                last_out_fd = fd[0];
                cmd = cmd->next; // process next command in next iter
            }
        }
        else {
            // we are in the child process
            // if there was a previous command, set its out to this one's input
            if (last_out_fd != -1) {
                printf("%s: setting in of this to be last_out_fd\n", cmd->exec_fn);
                dup2(last_out_fd, STDIN_FILENO);
                close(last_out_fd);
            }
            // if there is a next command, set this one's out to be pipe-input
            if (cmd->next != NULL) {
                close(fd[0]);
                printf("%s: setting out of this to be fd[1]\n", cmd->exec_fn);
                dup2(fd[1], STDOUT_FILENO);
                close(fd[1]);
            }
            printf("Executing: %s\n", cmd->exec_fn);
            execute_command(cmd);
            fprintf(stderr, "Failed to exec %s\n", cmd->exec_fn);
            return -1;
        }
    }

    // Make the shell process wait for all child processes to terminate
    wait(NULL);
    printf("all child processes are finished\n");

    return 0;
}

/* Executes the command after forking and does the io redirection to stdout, 
stdin, or stderr if needed. The io redirection is done after forking, so the 
redirection is unique to this process/command. Note: creat() is equivalent to 
open(path, O_WRONLY|O_CREAT|O_TRUNC, mode) and mode 0644 means write 
permissions are given to only the owner. */
void execute_command(struct command *cmd) {
    if (cmd->input_fn != NULL) {
        int fd0 = open(cmd->input_fn, O_RDONLY);
        dup2(fd0, STDIN_FILENO);
        close(fd0);
    }
    if (cmd->output_fn != NULL) {
        int fd1;

        // Append to output if out_a flag is 1, else make empty file
        if (cmd->out_a) {
            fd1 = open(cmd->output_fn, O_WRONLY|O_APPEND);
        } else {
            // 
            fd1 = creat(cmd->output_fn, 0644);        
        }
        dup2(fd1, STDOUT_FILENO);
        close(fd1);
    }
    if (cmd->error_fn != NULL) {
        int fd2;

        // Append to error if err_a flag is 1, else make empty file
        if (cmd->err_a) {
            fd2 = open(cmd->output_fn, O_WRONLY|O_APPEND);
        }
        else {
            fd2 = creat(cmd->error_fn, 0644);
        }
        dup2(fd2, STDERR_FILENO);
        close(fd2);
    }

    // Execute the command with its arguments
    execvp(cmd->exec_fn, cmd->argv);
    
    // The executable has control over the process now, or else:
    fprintf(stderr, "Failed to exec %s\n", cmd->exec_fn);
    exit(1);
}





/* Deprecated testing functions

int main(int argc, char *argv[]) {
    printf("test:\n");
    test_chained_commands();

    return 0;
}

void test_chained_commands() {
    char *args[3];
    args[0] = "echo";
    args[1] = "veb";
    args[2] = NULL;
    struct command first;
    first.exec_fn = "echo";
    first.argv = args;
    first.input_fn = NULL;
    first.output_fn = NULL;
    first.error_fn = NULL;
    first.next = NULL;

    char *args2[3];
    args2[0] = "grep";
    args2[1] = "v"; //"h";
    args2[2] = NULL;
    struct command second;
    second.exec_fn = "grep";
    second.argv = args2;
    second.input_fn = NULL;
    second.output_fn = "results.out";
    second.error_fn = "";
    second.next = NULL;

    first.next = &second;

    fork_and_exec_commands(&first);
}

void test_single_command() {
    char *args[3];
    args[0] = "wc";
    args[1] = NULL;
    args[2] = NULL;
    struct command cmd;
    cmd.exec_fn = "wc";
    cmd.argv = args;
    cmd.input_fn = "notes.out";
    cmd.output_fn = "count.out";
    cmd.error_fn = "";
    cmd.next = NULL;
    execute_command(&cmd);
}

void test_piping() {
    int n; // number of lines read/written from file
    int fd[2];
    pid_t pid; // pid of the child process
    char line[MAXLINE];

    if (pipe(fd) < 0)
        return -1; // pipe error
    if ((pid = fork()) < 0) {
        return -1; // fork error
    } else if (pid > 0) {
        // we are in the parent process
        close(fd[0]);
        write(fd[1], "hello world\n", 12);
    } else {
        // we are in the child preocess
        close(fd[1]);
        n = read(fd[0], line, MAXLINE);
        printf("number of bites read: %d\n", n);
        write(STDOUT_FILENO, line, n);
    }
}
*/




