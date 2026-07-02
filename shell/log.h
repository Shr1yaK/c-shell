#ifndef LOG_H
#define LOG_H

// add a command to the log
// won't add if it's identical to last command or if it's a log command
void log_add(const char *cmd);

// print all stored commands oldest to newest
void log_print();

// clear the log
void log_purge();

// execute command at given index (1 = newest, 2 = second newest etc)
// returns the command string to execute, NULL if invalid index
char *log_execute(int index);

#endif