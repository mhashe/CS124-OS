/*
 * The Breakfast Club, CS 124, Fall 2017
 * Vaibhav Anand, Nikhil Gupta, Michael Hashe
 *
 * Handles forking off and executing a linked list of command that are piped 
 * together and executed in parallel. It setups up redirection and pipes in 
 * the child processes (commands). It then waits for the processes to complete 
 * in the shell process.
 */

#include <unistd.h> // read, write
#include <stdio.h> // cout, cin
#include <stdlib.h>
#include <assert.h>
#include <fcntl.h> //open, creat
#include <sys/types.h> // pid_t, wait
#include <sys/wait.h> // wait
#include <errno.h>

#include "consts.h"
#include "com_parser.h"
#include "com_exec.h"
#include "com_int.h"


// TODO: Either 1. Remove debug info or 2. Make debug mode


int fork_and_exec_commands(struct command *cmd) {
    // stores the file descriptor of the read end of pipe created by the last 
    // command that was forked
    int last_out_fd = -1; // there was no previous fd to take as input

    // spawning processes in an infinite while loop seems dangerous: change?
    while (1) {
        // pid of the child process
        pid_t pid;

        // file descriptors containing read and write ends of a pipe
        int fd[2];

        // printf("Handling: %s\n", cmd->exec_fn);

        // handle command internally if it is internal 
        if (internal_command_handler(cmd, (cmd->next == NULL) && 
            (last_out_fd == -1))) {

            // if the handler returned 1, it was internal
            if (cmd->next != NULL) {

                // so prep for next command by iterating the cmd
                cmd = cmd->next;

                // and close the last pipe's read fd
                if (last_out_fd != -1) {
                    if (close(last_out_fd) == -1) {
                        perror("Error in closing a pipe's read end");
                        exit(1);
                    }

                    // before nullifying its existence
                    last_out_fd = -1;
                }
            } 
            else {
                // this was the last command to execute so don't bother,
                // forking off a new process. just return
                break;
            }
        }
        
        // if this is the last command, we do not need to create a pipe to a  
        // consecutive command
        if (cmd->next != NULL) {
            if (pipe(fd) == -1)
                perror("Pipe error");
                exit(1);
        }

        // execute the command in a forked child process (not shell process)
        if ((pid = fork()) < 0) {
            perror("Fork error");
            exit(1);
        }

        if (pid > 0) {
            // we are in the shell process, so prep for next command

            // close the read end of the last command's pipe if it exists
            if (last_out_fd != -1) {
                // printf("%s: closing last_out_fd\n", cmd->exec_fn);

                if (close(last_out_fd) == -1) {
                    perror("Error in closing a pipe's read end from shell");
                    exit(1);
                }
            }

            if (cmd->next == NULL) {
                // we are done forking all commands, so return
                break;

            } else {
                // there is a command after this, so a pipe exists, and we 
                // need to close it's write end
                if (close(fd[1]) == -1) {
                    perror("Error in closing a pipe's write end from shell");
                    exit(1);
                }

                // printf("%s: setting last_out_fd from fd[0], and going next\n", cmd->exec_fn);

                // save the pipe's read end into last_out_fd for the next
                // command
                last_out_fd = fd[0];

                // process the next command in the next iteration
                cmd = cmd->next;

            }
        }
        else {
            // we are in the child process, so setup redirection and execute cmd

            // if there was a previous command, set its output in last_out_fd 
            // to be the input of this and close its file descriptor
            if (last_out_fd != -1) {
                // printf("%s: setting in of this to be last_out_fd\n", cmd->exec_fn);

                if (dup2(last_out_fd, STDIN_FILENO) == -1) {
                    perror("Error in redirecting pipe's read-end file " 
                        "descriptor to stdin");
                    exit(1);
                }

                if (close(last_out_fd) == -1) {
                    perror("Error in closing a pipe's read end from child");
                    exit(1);
                }
            }
            // if there is a next command, set the output of this to be be 
            // redirected to the pipe's input
            if (cmd->next != NULL) {
                if (close(fd[0]) == -1) {
                    perror("Error in closing a pipe's read end from child");
                    exit(1);
                }

                // printf("%s: setting out of this to be fd[1]\n", cmd->exec_fn);

                if (dup2(fd[1], STDOUT_FILENO) == -1) {
                    perror("Error in redirecting pipe's write-end file " 
                        "descriptor to stdout");
                    exit(1);
                }

                if (close(fd[1]) == -1) {
                    perror("Error in closing a pipe's write end from child");
                    exit(1);
                }
            }

            // printf("Executing: %s\n", cmd->exec_fn);

            execute_command(cmd);

            return -1; // this line should never be reached
        }
    }

    // make the shell process wait for all child processes to terminate
    wait_for_children();

    // make sure everything form stdout has been received, so that the shell 
    // does receive any output from the forked commands it executed
    if (fflush(stdout) == EOF) {
        perror("Error in fflush(stdout) post command execution");
    }

    // printf("all child processes are finished\n");

    return 0;
}


void wait_for_children() {
    // continuously wait for child processes to terminate
    while (1) {
        int status;
        pid_t finished;

        // if we get the error code that no child processes are left, return
        if ((finished = wait(&status)) == -1) {
            if (errno == ECHILD) {
                return;
            }
            perror("Wait error");
            exit(1);
        }

        int exit_status = WEXITSTATUS(status);

        // only exit codes -1 (if cmd doesn't exist) or 0 (successful) are OK
        if ((exit_status != 0) && (exit_status != 1)) {
            fprintf(stderr, "Child exited with status: %d\n", exit_status);
        }
    }
}



void execute_command(struct command *cmd) {
    // If the cmd contains, input redirection from file, redirect it
    if (cmd->input_fn != NULL) {
        int fd0;

        if ((fd0 = open(cmd->input_fn, O_RDONLY)) == -1) {
            fprintf(stderr, "Error in open(%s) for command: %s\n", 
                cmd->input_fn, cmd->exec_fn);
            exit(1);
        }

        // replace the file descriptor of stdin with the file for redirection
        if (dup2(fd0, STDIN_FILENO) == -1) {
            fprintf(stderr, "Error in redirecting %s to stdin for " 
                "command: %s\n", cmd->input_fn, cmd->exec_fn);
            exit(1);
        }

        if (close(fd0) == -1) {
            fprintf(stderr, "Error in close(%s) for command: %s\n", 
                cmd->input_fn, cmd->exec_fn);
            exit(1);
        }
    }
    if (cmd->output_fn != NULL) {
        int fd1;

        // append to output if out_a flag is 1, else make an empty file
        if (cmd->out_a) {
            fd1 = open(cmd->output_fn, O_WRONLY|O_APPEND);
        } else {
            fd1 = creat(cmd->output_fn, 0644);        
        }

        if (fd1 == -1) {
            fprintf(stderr, "Error in open() or create of %s for command: %s\n", 
                cmd->output_fn, cmd->exec_fn);
            exit(1);
        }

        // replace the file descriptor of stdin with the file for redirection
        if (dup2(fd1, STDOUT_FILENO) == -1) {
            fprintf(stderr, "Error in redirecting %s to stdout for " 
                "command: %s\n", cmd->output_fn, cmd->exec_fn);
            exit(1);
        }

        if (close(fd1) == -1) {
            fprintf(stderr, "Error in close(%s) for command: %s\n", 
                cmd->output_fn, cmd->exec_fn);
            exit(1);
        }
    }
    if (cmd->error_fn != NULL) {
        int fd2;

        // append to error if err_a flag is 1, else make an empty file
        if (cmd->err_a) {
            fd2 = open(cmd->output_fn, O_WRONLY|O_APPEND);
        }
        else {
            fd2 = creat(cmd->error_fn, 0644);
        }

        if (fd2 == -1) {
            fprintf(stderr, "Error in open() or create of %s for command: %s\n", 
                cmd->error_fn, cmd->exec_fn);
            exit(1);
        }

        // replace the file descriptor of stdin with the file for redirection
        if (dup2(fd2, STDERR_FILENO) == -1) {
            fprintf(stderr, "Error in redirecting %s to stderr for " 
                "command: %s\n", cmd->error_fn, cmd->exec_fn);
            exit(1);
        }

        if (close(fd2) == -1) {
            fprintf(stderr, "Error in close(%s) for command: %s\n", 
                cmd->error_fn, cmd->exec_fn);
            exit(1);
        }
    }

    // execute the command with its arguments
    execvp(cmd->exec_fn, cmd->argv);
    
    // the executable has control over the process now, or else it failed and:
    fprintf(stderr, "%s: command not found or not executable\n", cmd->exec_fn);
    exit(1);
}

