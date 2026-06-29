#include "headers.h"
#include "prompt.h"
#include "parser.h"

char home_dir[PATH_MAX];
char hostname_str[256];
char username_str[256];

int main() {
    getcwd(home_dir, sizeof(home_dir));
    gethostname(hostname_str, sizeof(hostname_str));
    struct passwd *pw = getpwuid(getuid());
    strncpy(username_str, pw->pw_name, sizeof(username_str));

    char input[MAX_CMD_LEN];

    while (1) {
        print_prompt();

        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("\n");
            exit(0);
        }

        input[strcspn(input, "\n")] = '\0';
        if (strlen(input) == 0) continue;

        if (!is_valid_command(input)) {
            printf("Invalid Syntax!\n");
            continue;
        }

        printf("valid: %s\n", input);
    }

    return 0;
}