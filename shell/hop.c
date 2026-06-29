#include "headers.h"
#include "hop.h"

// static = private to this file only
// keeps its value between calls — this is how we track previous directory
char prev_dir_for_reveal[PATH_MAX] = "";

// helper — actually perform the directory change to target
static void do_hop(const char *target) {
    char cwd[PATH_MAX];
    getcwd(cwd, sizeof(cwd));  // save where we are NOW before changing

    if (chdir(target) == -1) {
        printf("No such directory!\n");
        return;
    }

    // update prev_dir to where we WERE before this hop
    strncpy(prev_dir_for_reveal, cwd, sizeof(prev_dir_for_reveal));
}

void hop(char **args, int arg_count) {
    // no arguments → go home
    if (arg_count == 1) {
        do_hop(home_dir);
        return;
    }

    // process each argument one by one
    for (int i = 1; i < arg_count; i++) {
        char *arg = args[i];

        if (strcmp(arg, "~") == 0) {
            // hop ~ → go home
            do_hop(home_dir);

        } else if (strcmp(arg, ".") == 0) {
            // hop . → stay here, do nothing
            continue;

        } else if (strcmp(arg, "..") == 0) {
            // hop .. → go up one level
            do_hop("..");

        } else if (strcmp(arg, "-") == 0) {
            // hop - → go to previous directory
            if (strlen(prev_dir_for_reveal) == 0) {
                // no previous directory yet, do nothing
                continue;
            }
            do_hop(prev_dir_for_reveal);

        } else {
            // hop somepath → go to that path (absolute or relative)
            do_hop(arg);
        }
    }
}