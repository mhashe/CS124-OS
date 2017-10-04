#include <stdio.h>

/* This simple program outputs "stdout" on stdout, and
 * "stderr" on stderr.
 */
int main() {
    printf("stdout\n");
    fprintf(stderr, "stderr\n");
    return 0;
}

