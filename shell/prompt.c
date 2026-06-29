#include "headers.h"
#include "prompt.h"

void print_prompt() {
    char cwd[PATH_MAX];
    getcwd(cwd, sizeof(cwd));

    char display_path[PATH_MAX];

    // if cwd starts with home_dir, replace that prefix with ~
    // e.g. /home/shriya/projects → ~/projects
    if (strncmp(cwd, home_dir, strlen(home_dir)) == 0) {
        snprintf(display_path, sizeof(display_path), "~%s", cwd + strlen(home_dir));
    } else {
        // outside home entirely, show full path
        strncpy(display_path, cwd, sizeof(display_path));
    }

    printf("<%s@%s:%s> ", username_str, hostname_str, display_path);
    fflush(stdout);  // force print immediately — no newline so buffer won't flush on its own
}