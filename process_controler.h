#ifndef _PROCESS_CONTROL_H_
#define _PROCESS_CONTROL_H_

#include <sys/types.h>


typedef struct{
    char name[16];
    int ready_time;
    int exec_time;
    pid_t pid;
    int pipe_fd[2];
}Process;

void inline TIME_UNIT(void);

int assign_core(pid_t pid, int core);

// create the process and pause
pid_t process_create(Process chld);

// make priority to low
int process_kickout(pid_t pid);

// make priority to high
int process_resume(pid_t pid);

// make to the hightest
int process_highest(pid_t pid);

#endif
