#define _GNU_SOURCE

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/syscall.h>

#include "process_controler.h"
#include "scheduler.h"

#define SYS_GETTIME 333
#define SYS_PRINT 334


void inline TIME_UNIT(void){
    volatile unsigned long i;
    for(i = 0; i < 1000000UL; i++);
}

//core will be 2 or 3
int assign_core(pid_t pid, int core){
    cpu_set_t cpu_mask;
    if (core > sizeof(cpu_mask)){
        fprintf(stderr, "Core Number %d is bigger than the number of core %d you have\n", core, sizeof(cpu_mask));
        return -1;
    }
    CPU_ZERO(&cpu_mask);
    CPU_SET(core, &cpu_mask);
    if ( sched_setaffinity(pid, sizeof(cpu_mask), &cpu_mask) != 0 ){
        perror("error: sched_setaffinity can't fit CPU");
        exit(-1);
    }
    return 0;
}

// parent create the process and pause it
pid_t process_create(Process chld){
    pid_t chpid = fork();
    if ( chpid < 0 ){
        perror("error: fork");
        exit(2);
    }
    //fork can inherit pointer copy
    if ( chpid == 0 ){ 
        close( chld.pipe_fd[1] );
        int init_exec_time = chld.exec_time;
        long start, end;
        while( chld.exec_time > 0 ){
            char buf[8];
            read(chld.pipe_fd[0], buf, strlen("run"));
            //mean the first time fork
            if( chld.exec_time == init_exec_time ){
                start = syscall(SYS_GETTIME);
                printf("%s %d\n", chld.name, getpid());
            }
            TIME_UNIT();
            chld.exec_time--;
        }
        end = syscall(SYS_GETTIME);
        syscall(SYS_PRINT, getpid() , start, end);
        exit(0);
        return -1;
    }
    //parent will do kill it out first
    else{
        process_kickout(chpid);
        assign_core(chpid, CHILD_CORE);
        close( chld.pipe_fd[0] );
        return chpid;
    }
}

// set pid to lower priority 
int process_kickout(pid_t pid){
    struct sched_param param;
    // normal policy only can only use 0
    param.sched_priority = 0;
    // set process to the very low priority (IDLE)
    if ( sched_setscheduler(pid, SCHED_IDLE, &param) < 0 ){
        perror("error: sched_setscheduler can't idle");
        return -1;
    }
    return 0;
}

// set pid to OTHER priority group 
// or set to real policy
int process_resume(pid_t pid){
    struct sched_param param;
    //param.sched_priority = 0;
    param.sched_priority = sched_get_priority_max(SCHED_FIFO);
    //RR time sharing
    //if ( sched_setscheduler(pid, SCHED_OTHER, &param) < 0 ){
    if ( sched_setscheduler(pid, SCHED_FIFO, &param) < 0 ){
        perror("error: sched_setscheduler can't max");
        return -1;
    }
    return 0;
}
