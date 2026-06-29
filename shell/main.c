#include "headers.h"
#include "prompt.h"
#include "parser.h"
#include "hop.h"

char home_dir[PATH_MAX];
char hostname_str[256];
char username_str[256];

// splits input string into array of words
// "hop ~/foo" → ["hop", "~/foo", NULL]
char **tokenise(char *input, int *count) {
    char **args = malloc(256 * sizeof(char *));
    *count = 0;
    char *copy = strdup(input);        // strtok modifies the string so copy it first
    char *token = strtok(copy, " \t");
    while (token != NULL) {
        args[(*count)++] = token;
        token = strtok(NULL, " \t");
    }
    args[*count] = NULL;
    return args;
}

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

        int count = 0;
        char **args = tokenise(input, &count);

        if (strcmp(args[0], "hop") == 0) {
            hop(args, count);
        } else {
            printf("valid: %s\n", input);
        }

        free(args[0]);   // free the strdup copy
        free(args);
    }

    return 0;
}