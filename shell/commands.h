#ifndef COMMANDS_H
#define COMMANDS_H

// executes a single external command with its arguments
// e.g. args = ["cat", "file.txt", NULL]
void execute_command(char **args, int arg_count);

#endif