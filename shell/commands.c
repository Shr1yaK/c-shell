#include "headers.h"
#include "commands.h"

void execute_command(char **args, int arg_count) {
    pid_t pid = fork();

    if (pid == 0) {
        // CHILD — become the external command
        execvp(args[0], args);

        // if execvp returns, the command wasn't found
        printf("Command not found!\n");
        exit(1);

    } else if (pid > 0) {
        // PARENT — wait for child to finish
        waitpid(pid, NULL, 0);

    } else {
        // fork itself failed (very rare)
        perror("fork");
    }
}