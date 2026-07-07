#include "headers.h"
#include "activities.h"
#include <sys/wait.h>

// access the bg_jobs array from execute.c
typedef struct {
    pid_t pid;
    int   job_number;
    char  name[256];
    int   stopped;  // 0 = running, 1 = stopped
} ActivityJob;

extern ActivityJob bg_jobs[];
extern int bg_job_count;

// compare function for qsort — lexicographic by name
static int cmp_jobs(const void *a, const void *b) {
    return strcmp(((ActivityJob *)a)->name, ((ActivityJob *)b)->name);
}

void activities() {
    if (bg_job_count == 0) return;

    // make a copy to sort without modifying the original
    ActivityJob sorted[64];
    for (int i = 0; i < bg_job_count; i++) {
        sorted[i] = bg_jobs[i];
    }

    qsort(sorted, bg_job_count, sizeof(ActivityJob), cmp_jobs);

    for (int i = 0; i < bg_job_count; i++) {
        printf("[%d] : %s - %s\n",
               sorted[i].pid,
               sorted[i].name,
               sorted[i].stopped ? "Stopped" : "Running");
    }
}