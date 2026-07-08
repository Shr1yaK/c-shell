#include "headers.h"
#include "fgbg.h"
#include "signals.h"
#include <signal.h>

// access bg_jobs from execute.c
typedef struct {
    pid_t pid;
    int   job_number;
    char  name[256];
    int   stopped;
} ActivityJob;

extern ActivityJob bg_jobs[];
extern int bg_job_count;

void fg(char **args, int arg_count) {
    if (bg_job_count == 0) {
        printf("No such job\n");
        return;
    }

    int target_job = -1;

    if (arg_count == 1) {
        // no job number — use most recently added job
        target_job = bg_job_count - 1;
    } else {
        // find job by job number
        int job_num = atoi(args[1]);
        for (int i = 0; i < bg_job_count; i++) {
            if (bg_jobs[i].job_number == job_num) {
                target_job = i;
                break;
            }
        }
    }

    if (target_job == -1) {
        printf("No such job\n");
        return;
    }

    ActivityJob *job = &bg_jobs[target_job];

    // print the command being brought to foreground
    printf("%s\n", job->name);

    // set as foreground process group
    foreground_pgid = job->pid;

    // if stopped, send SIGCONT to resume it
    if (job->stopped) {
        job->stopped = 0;
        killpg(job->pid, SIGCONT);
    }

    // remove from bg_jobs — it's now foreground
    for (int i = target_job; i < bg_job_count - 1; i++) {
        bg_jobs[i] = bg_jobs[i + 1];
    }
    bg_job_count--;

    // wait for it to finish or stop again
    int status;
    waitpid(foreground_pgid, &status, WUNTRACED);

    if (WIFSTOPPED(status)) {
        // stopped again — add back to bg_jobs as stopped
        bg_jobs[bg_job_count].pid        = foreground_pgid;
        bg_jobs[bg_job_count].stopped    = 1;
        strncpy(bg_jobs[bg_job_count].name, job->name,
                sizeof(bg_jobs[bg_job_count].name));
        bg_job_count++;
    }

    foreground_pgid = -1;
}

void bg(char **args, int arg_count) {
    if (bg_job_count == 0) {
        printf("No such job\n");
        return;
    }

    int target_job = -1;

    if (arg_count == 1) {
        // no job number — use most recently added job
        target_job = bg_job_count - 1;
    } else {
        int job_num = atoi(args[1]);
        for (int i = 0; i < bg_job_count; i++) {
            if (bg_jobs[i].job_number == job_num) {
                target_job = i;
                break;
            }
        }
    }

    if (target_job == -1) {
        printf("No such job\n");
        return;
    }

    ActivityJob *job = &bg_jobs[target_job];

    if (!job->stopped) {
        printf("Job already running\n");
        return;
    }

    // resume in background
    job->stopped = 0;
    killpg(job->pid, SIGCONT);

    printf("[%d] %s &\n", job->job_number, job->name);
}