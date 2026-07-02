#include "headers.h"
#include "execute.h"
#include "commands.h"
#include "hop.h"
#include "reveal.h"

// background job tracking
typedef struct {
    pid_t pid;
    int   job_number;
    char  name[256];
} BackgroundJob;

static BackgroundJob bg_jobs[64];
static int bg_job_count = 0;
static int next_job_number = 1;

// check if any background jobs have finished
// called before printing each prompt
void check_background_jobs() {
    for (int i = 0; i < bg_job_count; i++) {
        int status;
        pid_t result = waitpid(bg_jobs[i].pid, &status, WNOHANG);

        if (result == 0) continue;  // still running

        if (result > 0) {
            if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                printf("%s with pid %d exited normally\n",
                       bg_jobs[i].name, bg_jobs[i].pid);
            } else {
                printf("%s with pid %d exited abnormally\n",
                       bg_jobs[i].name, bg_jobs[i].pid);
            }

            // remove this job from the list by shifting
            for (int j = i; j < bg_job_count - 1; j++) {
                bg_jobs[j] = bg_jobs[j + 1];
            }
            bg_job_count--;
            i--;  // recheck this index since we shifted
        }
    }
}

// tokenise a single command string into args array
static char **tokenise_cmd(char *cmd, int *count) {
    char **args = malloc(256 * sizeof(char *));
    *count = 0;
    char *copy = strdup(cmd);
    char *token = strtok(copy, " \t");
    while (token != NULL) {
        args[(*count)++] = token;
        token = strtok(NULL, " \t");
    }
    args[*count] = NULL;
    return args;
}

// run a single cmd_group — dispatches to hop, reveal, or execute_command
static void run_cmd_group(char *cmd, int background) {
    // trim leading whitespace
    while (*cmd == ' ' || *cmd == '\t') cmd++;

    if (strlen(cmd) == 0) return;

    int count = 0;
    char **args = tokenise_cmd(cmd, &count);
    if (count == 0) { free(args); return; }

    if (strcmp(args[0], "hop") == 0) {
        hop(args, count);
        free(args[0]);
        free(args);
        return;
    }

    if (strcmp(args[0], "reveal") == 0) {
        reveal(args, count);
        free(args[0]);
        free(args);
        return;
    }

    if (background) {
        // fork without waiting
        pid_t pid = fork();
        if (pid == 0) {
            // child — execute the command
            execute_command(args, count);
            exit(0);
        } else if (pid > 0) {
            // parent — don't wait, just record the job
            bg_jobs[bg_job_count].pid = pid;
            bg_jobs[bg_job_count].job_number = next_job_number;
            strncpy(bg_jobs[bg_job_count].name, args[0],
                    sizeof(bg_jobs[bg_job_count].name));
            printf("[%d] %d\n", next_job_number, pid);
            next_job_number++;
            bg_job_count++;
        }
    } else {
        // foreground — run and wait
        execute_command(args, count);
    }

    free(args[0]);
    free(args);
}

// splits input on ; and & and runs each piece
void execute(char *input) {
    char *copy = strdup(input);
    char *pos  = copy;

    while (*pos != '\0') {
        // find next ; or &
        char *sep = strpbrk(pos, ";&");

        if (sep == NULL) {
            // no more separators — run the rest as foreground
            run_cmd_group(pos, 0);
            break;
        }

        char separator = *sep;
        *sep = '\0';  // null-terminate this piece

        if (separator == '&') {
            run_cmd_group(pos, 1);  // background
        } else {
            run_cmd_group(pos, 0);  // sequential, foreground
        }

        pos = sep + 1;
    }

    free(copy);
}