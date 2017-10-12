#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


/* The size of the buffer to use for reading and writing. */
#define BUFFER_SIZE 4096

/* The buffer used for reading and writing. */
char buf[BUFFER_SIZE];


/* This helper function sends the data from the specified file descriptor
 * out to stdout.
 */
void send_to_stdout(int fd) {
    while (1) {
        int count = read(fd, buf, BUFFER_SIZE);
        if (count == 0)
            break;

        write(STDOUT_FILENO, buf, count);
    }
}


/* This program behaves like the "cat" utility, except that it sleeps
 * for a specified number of seconds before doing so.
 * This way we can make sure that processes and pipes behave correctly.
 */
int main(int argc, char **argv) {
    unsigned int secs;

    if (argc < 2) {
        printf("usage:  %s secs [file1 [file2 [...]]]\n", argv[0]);
        printf("\tSleeps for specified number of seconds, then behaves like"
               " cat\n");
        return 1;
    }

    /* Sleep! */
    /* TODO:  Detect parse errors! */
    secs = (unsigned int) atoi(argv[1]);
    sleep(secs);

    if (argc == 2) {
        /* Read from stdin and feed it to stdout. */
        send_to_stdout(STDIN_FILENO);
    }
    else {
        int i, fd;

        /* Open each file, send it to stdout, then close it. */
        for (i = 2; i < argc; i++) {
            fd = open(argv[i], O_RDONLY);
            if (fd == -1) {
                fprintf(stderr, "Couldn't open file \"%s\":  %s\n", argv[i],
                        strerror(fd));
                return 2;
            }
            send_to_stdout(fd);
            close(fd);
        }
    }

    return 0;
}

