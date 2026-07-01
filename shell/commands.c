#include "headers.h"
#include "commands.h"
#include <fcntl.h>

// parses args to find redirections and the actual command args
// fills in input_file, output_file, append flag
// returns a new args array with just the command and its arguments (no < > filenames)
static char **parse_redirections(char **args, int arg_count,
                                  char **input_file, char **output_file, int *append) {
    *input_file  = NULL;
    *output_file = NULL;
    *append      = 0;

    char **cmd_args = malloc(256 * sizeof(char *));
    int cmd_count = 0;

    for (int i = 0; i < arg_count; i++) {
        if (strcmp(args[i], "<") == 0) {
            // next token is the input file
            if (i + 1 < arg_count) {
                *input_file = args[++i];
            }
        } else if (strcmp(args[i], ">>") == 0) {
            // next token is the output file, append mode
            if (i + 1 < arg_count) {
                *output_file = args[++i];
                *append = 1;
            }
        } else if (strcmp(args[i], ">") == 0) {
            // next token is the output file, overwrite mode
            if (i + 1 < arg_count) {
                *output_file = args[++i];
                *append = 0;
            }
        } else {
            // regular argument — keep it
            cmd_args[cmd_count++] = args[i];
        }
    }
    cmd_args[cmd_count] = NULL;
    return cmd_args;
}

void execute_command(char **args, int arg_count) {
    char *input_file  = NULL;
    char *output_file = NULL;
    int   append      = 0;

    char **cmd_args = parse_redirections(args, arg_count,
                                         &input_file, &output_file, &append);

    pid_t pid = fork();

    if (pid == 0) {
        // CHILD

        // C.2 — input redirection
        if (input_file != NULL) {
            int fd = open(input_file, O_RDONLY);
            if (fd == -1) {
                printf("No such file or directory\n");
                free(cmd_args);
                exit(1);
            }
            dup2(fd, STDIN_FILENO);   // stdin now reads from file
            close(fd);                // close original fd, don't need it anymore
        }

        // C.3 — output redirection
        if (output_file != NULL) {
            int flags = O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC);
            int fd = open(output_file, flags, 0644);
            if (fd == -1) {
                printf("Unable to create file for writing\n");
                free(cmd_args);
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);  // stdout now writes to file
            close(fd);
        }

        // become the command
        execvp(cmd_args[0], cmd_args);
        printf("Command not found!\n");
        exit(1);

    } else if (pid > 0) {
        // PARENT — wait for child
        waitpid(pid, NULL, 0);

    } else {
        perror("fork");
    }

    free(cmd_args);
}