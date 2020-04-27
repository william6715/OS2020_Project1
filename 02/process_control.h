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

// defines one unit of time
// one time unit is the time to run this function once
void inline TIME_UNIT(void);

int assign_core(pid_t pid, int core);

// spawn new process when ready_time is reached
pid_t process_create(Process chld);

// kick child out of core
int process_kickout(pid_t pid);

// bring another child to run
int process_resume(pid_t pid);

#endif
