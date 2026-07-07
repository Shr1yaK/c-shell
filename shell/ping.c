#include "headers.h"
#include "ping.h"
#include <signal.h>

void ping(char **args, int arg_count) {
    if (arg_count != 3) {
        printf("Invalid syntax!\n");
        return;
    }

    // validate pid
    char *pid_end;
    long pid = strtol(args[1], &pid_end, 10);
    if (*pid_end != '\0') {
        printf("Invalid syntax!\n");
        return;
    }

    // validate signal number
    char *sig_end;
    long signal_num = strtol(args[2], &sig_end, 10);
    if (*sig_end != '\0') {
        printf("Invalid syntax!\n");
        return;
    }

    // take modulo 32
    int actual_signal = signal_num % 32;

    // send the signal
    if (kill((pid_t)pid, actual_signal) == -1) {
        printf("No such process found\n");
        return;
    }

    printf("Sent signal %ld to process with pid %ld\n", signal_num, pid);
}