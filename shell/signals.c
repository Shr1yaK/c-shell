#include "headers.h"
#include "signals.h"
#include "execute.h"
#include <signal.h>

// defined here, declared extern in signals.h
pid_t foreground_pgid = -1;

// access bg_jobs from execute.c
typedef struct {
    pid_t pid;
    int   job_number;
    char  name[256];
    int   stopped;
} ActivityJob;

extern ActivityJob bg_jobs[];
extern int bg_job_count;
extern int next_job_number;

// current foreground command name (set in execute.c before forking)
char foreground_cmd_name[256] = "";

// SIGINT handler — Ctrl-C
// send SIGINT to foreground process group, shell survives
static void handle_sigint(int sig) {
    (void)sig;
    if (foreground_pgid > 0) {
        killpg(foreground_pgid, SIGINT);
    }
    // if no foreground process, do nothing — shell keeps running
}

// SIGTSTP handler — Ctrl-Z
// stop the foreground process, move it to background as Stopped
static void handle_sigtstp(int sig) {
    (void)sig;
    if (foreground_pgid > 0) {
        killpg(foreground_pgid, SIGTSTP);

        // add to bg_jobs as stopped
        bg_jobs[bg_job_count].pid        = foreground_pgid;
        bg_jobs[bg_job_count].job_number = next_job_number;
        bg_jobs[bg_job_count].stopped    = 1;
        strncpy(bg_jobs[bg_job_count].name, foreground_cmd_name,
                sizeof(bg_jobs[bg_job_count].name));

        printf("\n[%d] Stopped %s\n", next_job_number, foreground_cmd_name);
        fflush(stdout);

        next_job_number++;
        bg_job_count++;

        foreground_pgid = -1;  // no foreground process anymore
    }
}

void setup_signal_handlers() {
    // install handlers
    signal(SIGINT,  handle_sigint);
    signal(SIGTSTP, handle_sigtstp);

    // ignore SIGTTOU so shell can write to terminal while in background
    signal(SIGTTOU, SIG_IGN);
}