#include <unistd.h> // read, write
#include <stdio.h> // cout, cin
#include <stdlib.h>
#include <assert.h>

#include "consts.h"
#include "com_parser.h"
#include "com_exec.h"

/* 
 * Handles forking off and executing a linked list of command that are piped 
 * together and executed in parrallel. It setups up redirection and pipes in 
 * the child processes (commands). It then waits for the processes to complete 
 * in the shell process.
 */


/* Deprecated testing functions

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

int main(int argc, char *argv[]) {
    printf("test:\n");
    // test_piping();

    return 0;
}





