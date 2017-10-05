#include <stdlib.h>
#include <pwd.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "token.h"
#include "consts.h"
#include "com_parser.h"
#include "com_exec.h"


int main(int argc, char *argv[])
{
    // User ID. Function guaranteed to be successful.
    uid_t uid = getuid();

    // Struct containing user information
    struct passwd* pw = getpwuid(uid);
    if (!pw)
    {
        perror("Error in getpwuid");
        return 1;
    }
    // User name, home directory
    char* uname = pw->pw_name;
    char* hdir = pw->pw_dir;

    // Path length at most PATH_MAX
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("Error in getcwd");
        return 1;
    }

    // If process in home directory of user, change prompt
    if (strstr(cwd, hdir) != NULL) {
        // Copy to hold cwd
        char copy[PATH_MAX];
        strcpy(copy, cwd); // No return value for error conditions

        // Replace /home/USER with ~
        cwd[0] = '~';
        memmove(&cwd[1], &copy[strlen(hdir)], PATH_MAX - strlen(hdir));
    }

    while(1) {
        char *line_in;

        // Print command prompt
        // printf("%s:%s $ ", uname, cwd);

        // read input
        line_in = readline("prompt");

        // Wait for input
        if (line_in == NULL) {
            // Received EOF => stdin is closed, no reason for terminal
            printf("\n");
            return 0;
        }

        add_history(line_in);
        HIST_ENTRY* hist = previous_history();
        if (hist) {
            printf("%s\n", hist->line);
            next_history();
        }
        

        // Tokenize input
        char** comms = tokenize(line_in);


        // char* comm;
        // int i = 0;
        // while(1) {
        //     comm = comms[i];
        //     if (comm == NULL) {
        //         break;
        //     }
        //     printf("%s\n", comm);
        //     i++;

        // }

        struct command* cmd = parse_to_chained_commands(comms);
        char **cmds;
        int idx;

        while (cmd != NULL) {
            cmds = cmd->argv;

            idx = 0;
            while (cmds[idx] != NULL) {
                printf("%s\n",cmds[idx]);
                idx += 1;
            }
            printf("=====\n");
            if (cmd->input_fn != NULL) {
                printf("%s\n", cmd->input_fn);
            }
            if (cmd->output_fn != NULL) {
                printf("%s\n", cmd->output_fn);
            }

            cmd = cmd->next;
            printf("\n");
        }

        printf("forking result: %d\n", fork_and_exec_commands(cmd));

        // TODO: Free statements
    }

    return 0;
}