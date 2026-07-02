#include "headers.h"
#include "log.h"

#define MAX_LOG 15
#define LOG_FILE "/.shell_log"  // will be appended to home_dir

// get full path to log file e.g. /home/shriya/.shell_log
static void get_log_path(char *path) {
    snprintf(path, PATH_MAX, "%s%s", home_dir, LOG_FILE);
}

// read all commands from file into array
// returns number of commands read
static int read_log(char cmds[MAX_LOG][MAX_CMD_LEN]) {
    char path[PATH_MAX];
    get_log_path(path);

    FILE *f = fopen(path, "r");
    if (f == NULL) return 0;  // no log file yet

    int count = 0;
    while (count < MAX_LOG && fgets(cmds[count], MAX_CMD_LEN, f) != NULL) {
        // strip trailing newline
        cmds[count][strcspn(cmds[count], "\n")] = '\0';
        count++;
    }
    fclose(f);
    return count;
}

// write all commands back to file
static void write_log(char cmds[MAX_LOG][MAX_CMD_LEN], int count) {
    char path[PATH_MAX];
    get_log_path(path);

    FILE *f = fopen(path, "w");
    if (f == NULL) return;

    for (int i = 0; i < count; i++) {
        fprintf(f, "%s\n", cmds[i]);
    }
    fclose(f);
}

void log_add(const char *cmd) {
    // don't store log commands
    if (strncmp(cmd, "log", 3) == 0 && (cmd[3] == '\0' || cmd[3] == ' '))
        return;

    char cmds[MAX_LOG][MAX_CMD_LEN];
    int count = read_log(cmds);

    // don't store if identical to last command
    if (count > 0 && strcmp(cmds[count - 1], cmd) == 0)
        return;

    if (count < MAX_LOG) {
        // still have space — just add
        strncpy(cmds[count], cmd, MAX_CMD_LEN);
        count++;
    } else {
        // full — shift everything left (drop oldest), add at end
        for (int i = 0; i < MAX_LOG - 1; i++) {
            strncpy(cmds[i], cmds[i + 1], MAX_CMD_LEN);
        }
        strncpy(cmds[MAX_LOG - 1], cmd, MAX_CMD_LEN);
    }

    write_log(cmds, count);
}

void log_print() {
    char cmds[MAX_LOG][MAX_CMD_LEN];
    int count = read_log(cmds);

    if (count == 0) return;  // nothing to print

    // print oldest to newest
    for (int i = 0; i < count; i++) {
        printf("%s\n", cmds[i]);
    }
}

void log_purge() {
    char path[PATH_MAX];
    get_log_path(path);

    // just overwrite with empty file
    FILE *f = fopen(path, "w");
    if (f != NULL) fclose(f);
}

// returns command at given index (1 = newest, 2 = second newest)
// caller must free the returned string
char *log_execute(int index) {
    char cmds[MAX_LOG][MAX_CMD_LEN];
    int count = read_log(cmds);

    if (index < 1 || index > count) {
        printf("log: index out of range\n");
        return NULL;
    }

    // index 1 = newest = cmds[count-1]
    int actual = count - index;
    return strdup(cmds[actual]);
}