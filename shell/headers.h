#ifndef HEADERS_H
#define HEADERS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>
#include <limits.h>
#include <ctype.h>

#define MAX_CMD_LEN 4096

// these are defined ONCE in main.c
// every other file uses them via extern
extern char home_dir[PATH_MAX];
extern char hostname_str[256];
extern char username_str[256];

#endif