#include "headers.h"
#include "reveal.h"
#include <dirent.h>

extern char prev_dir_for_reveal[PATH_MAX];

static int resolve_path(const char *arg, char *resolved) {
    if (strcmp(arg, "~") == 0) {
        strncpy(resolved, home_dir, PATH_MAX);
    } else if (strcmp(arg, ".") == 0) {
        getcwd(resolved, PATH_MAX);
    } else if (strcmp(arg, "..") == 0) {
        char cwd[PATH_MAX];
        getcwd(cwd, sizeof(cwd));
        char *last = strrchr(cwd, '/');
        if (last == cwd) {
            strncpy(resolved, "/", PATH_MAX);
        } else {
            *last = '\0';
            strncpy(resolved, cwd, PATH_MAX);
        }
    } else if (strcmp(arg, "-") == 0) {
        // - with no previous hop = invalid
        if (strlen(prev_dir_for_reveal) == 0) {
            resolved[0] = '\0';
            return 0;  // signal failure
        }
        strncpy(resolved, prev_dir_for_reveal, PATH_MAX);
    } else {
        strncpy(resolved, arg, PATH_MAX);
    }
    return 1;
}

static int cmp_names(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

static void list_directory(const char *path, int show_hidden, int long_format) {
    DIR *dir = opendir(path);
    if (dir == NULL) {
        printf("No such directory!\n");
        return;
    }

    char *entries[4096];
    int count = 0;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        if (!show_hidden && entry->d_name[0] == '.')
            continue;
        entries[count++] = strdup(entry->d_name);
    }
    closedir(dir);

    qsort(entries, count, sizeof(char *), cmp_names);

    if (long_format) {
        for (int i = 0; i < count; i++) {
            printf("%s\n", entries[i]);
            free(entries[i]);
        }
    } else {
        for (int i = 0; i < count; i++) {
            printf("%s", entries[i]);
            if (i < count - 1) printf(" ");
            free(entries[i]);
        }
        if (count > 0) printf("\n");
    }
}

void reveal(char **args, int arg_count) {
    int show_hidden = 0;
    int long_format = 0;
    char path[PATH_MAX];
    int path_set = 0;
    int flag_count = 0;  // track how many flag groups we've seen

    for (int i = 1; i < arg_count; i++) {
        if (args[i][0] == '-' && args[i][1] != '\0') {
            // only allow ONE flag group e.g. -la
            // a second flag group like -la -a is invalid
            if (flag_count > 0 || path_set) {
                printf("reveal: Invalid Syntax!\n");
                return;
            }
            flag_count++;
            for (int j = 1; args[i][j] != '\0'; j++) {
                if (args[i][j] == 'a') show_hidden = 1;
                else if (args[i][j] == 'l') long_format = 1;
                else {
                    printf("reveal: Invalid Syntax!\n");
                    return;
                }
            }
        } else {
            if (path_set) {
                printf("reveal: Invalid Syntax!\n");
                return;
            }
            int ok = resolve_path(args[i], path);
            if (!ok) {
                printf("No such directory!\n");
                return;
            }
            path_set = 1;
        }
    }

    if (!path_set) {
        getcwd(path, sizeof(path));
    }

    list_directory(path, show_hidden, long_format);
}