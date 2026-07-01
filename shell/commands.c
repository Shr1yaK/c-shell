#include "headers.h"
#include "commands.h"
#include <fcntl.h>

// parses a single command's args to find < and > redirections
// returns clean args array with just the command and its arguments
static char **parse_redirections(char **args, int arg_count,
                                  char **input_file, char **output_file, int *append) {
    *input_file  = NULL;
    *output_file = NULL;
    *append      = 0;

    char **cmd_args = malloc(256 * sizeof(char *));
    int cmd_count = 0;

    for (int i = 0; i < arg_count; i++) {
        if (strcmp(args[i], "<") == 0) {
            if (i + 1 < arg_count) *input_file = args[++i];
        } else if (strcmp(args[i], ">>") == 0) {
            if (i + 1 < arg_count) { *output_file = args[++i]; *append = 1; }
        } else if (strcmp(args[i], ">") == 0) {
            if (i + 1 < arg_count) { *output_file = args[++i]; *append = 0; }
        } else {
            cmd_args[cmd_count++] = args[i];
        }
    }
    cmd_args[cmd_count] = NULL;
    return cmd_args;
}

// splits args array on | into separate commands
// e.g. ["ls", "|", "grep", "foo", "|", "wc"] 
// → cmds[0] = ["ls"], cmds[1] = ["grep", "foo"], cmds[2] = ["wc"]
static int split_on_pipe(char **args, int arg_count,
                          char **cmds[], int cmd_counts[]) {
    int num_cmds = 0;
    int start = 0;

    for (int i = 0; i <= arg_count; i++) {
        if (i == arg_count || strcmp(args[i], "|") == 0) {
            int len = i - start;
            char **cmd = malloc((len + 1) * sizeof(char *));
            for (int j = 0; j < len; j++) cmd[j] = args[start + j];
            cmd[len] = NULL;
            cmds[num_cmds] = cmd;
            cmd_counts[num_cmds] = len;
            num_cmds++;
            start = i + 1;
        }
    }
    return num_cmds;
}

void execute_command(char **args, int arg_count) {
    // split on | to get individual commands in the pipeline
    char **cmds[64];
    int cmd_counts[64];
    int num_cmds = split_on_pipe(args, arg_count, cmds, cmd_counts);

    // create all pipes upfront — we need (num_cmds - 1) pipes
    // pipes[i] connects command i to command i+1
    int pipes[64][2];
    for (int i = 0; i < num_cmds - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            return;
        }
    }

    pid_t pids[64];

    for (int i = 0; i < num_cmds; i++) {
        char **cmd = cmds[i];
        int count  = cmd_counts[i];

        // parse redirections for this specific command
        char *input_file  = NULL;
        char *output_file = NULL;
        int   append      = 0;
        char **cmd_args = parse_redirections(cmd, count,
                                             &input_file, &output_file, &append);

        pids[i] = fork();

        if (pids[i] == 0) {
            // CHILD i

            // if not the first command, read from previous pipe
            if (i > 0) {
                dup2(pipes[i-1][0], STDIN_FILENO);
            }

            // if not the last command, write to current pipe
            if (i < num_cmds - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }

            // close ALL pipe ends in child — we've already dup2'd what we need
            for (int j = 0; j < num_cmds - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            // handle file redirections (overrides pipe if both present)
            if (input_file != NULL) {
                int fd = open(input_file, O_RDONLY);
                if (fd == -1) {
                    printf("No such file or directory\n");
                    exit(1);
                }
                dup2(fd, STDIN_FILENO);
                close(fd);
            }

            if (output_file != NULL) {
                int flags = O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC);
                int fd = open(output_file, flags, 0644);
                if (fd == -1) {
                    printf("Unable to create file for writing\n");
                    exit(1);
                }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            execvp(cmd_args[0], cmd_args);
            printf("Command not found!\n");
            exit(1);
        }

        free(cmd_args);
        free(cmd);
    }

    // PARENT — close all pipe ends
    // critical: if parent keeps write end open, next command waits forever
    for (int i = 0; i < num_cmds - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    // wait for all children to finish
    for (int i = 0; i < num_cmds; i++) {
        waitpid(pids[i], NULL, 0);
    }
}