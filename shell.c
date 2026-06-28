#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <limits.h>

char home_dir[PATH_MAX];

void print_prompt() {
    char hostname[256];
    char cwd[PATH_MAX];
    
    // get username from the OS (looks up /etc/passwd for current user's entry)
    struct passwd *pw = getpwuid(getuid());
    char *username = pw->pw_name;
    
    // get machine hostname
    gethostname(hostname, sizeof(hostname));
    
    // get current working directory (full absolute path)
    getcwd(cwd, sizeof(cwd));
    
    // now ~ substitution
    char display_path[PATH_MAX];
    
    if (strncmp(cwd, home_dir, strlen(home_dir)) == 0) {
        // cwd starts with home_dir so replace home prefix with ~
        snprintf(display_path, sizeof(display_path), "~%s", cwd + strlen(home_dir));
    } else {
        // completely outside home, show full path
        strncpy(display_path, cwd, sizeof(display_path));
    }
    
    printf("<%s@%s:%s> ", username, hostname, display_path);
    fflush(stdout);  // force to print immediately, no buffering
}

int main() {
    // save home dir as wherever shell was launched from
    getcwd(home_dir, sizeof(home_dir));
    
    char input[4096];
    
    while (1) {
        print_prompt();
        
        if (fgets(input, sizeof(input), stdin) == NULL) {
            // user pressed Ctrl+D (EOF)
            printf("\n");
            exit(0);
        }
        
        // strip the newline fgets leaves at the end
        input[strcspn(input, "\n")] = '\0';
        
        if (strlen(input) == 0) continue;  // empty enter, just reprint prompt
        printf("got: %s\n", input);
    }
    
    return 0;
}