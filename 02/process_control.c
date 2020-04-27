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

#include "process_control.h"
#include "scheduler.h"

#define SYS_GETTIME 333
#define SYS_PRINT 334


typedef struct sched_param Sched_pm;
typedef struct timespec Time_sp;


void inline TIME_UNIT(void){
    volatile unsigned long i;
    for(i = 0; i < 1000000UL; i++);
    return;
}

/* this function makes pid only run on core */
int assign_core(pid_t pid, int core){
    cpu_set_t cpu_mask;

    /* it's an error if core is greater than 
       the number of cpus permitted by the system */
    if (core > sizeof(cpu_mask)){
        fprintf(stderr, "Invalid Core No.: %d\n", core);
        return -1;
    }

    /* clear cpu_mask */
    CPU_ZERO(&cpu_mask);
    /* set the flag on cpu */
    CPU_SET(core, &cpu_mask);

    /* assign pid on cpu indicated by core */
    if ( sched_setaffinity(pid, sizeof(cpu_mask), &cpu_mask) != 0 ){
        perror("error: sched_setaffinity");
        exit(-1);
    }

    return 0;
}

/* function used to create and run child process */
pid_t proc_create(Process chld){
    pid_t chpid = fork();
    if ( chpid < 0 ){
        perror("error: fork");
        exit(2);
    }
    if ( chpid == 0 ){ 
        /* close the pipe write file descriptor for the child */
        close( chld.pipe_fd[1] );
        
        /* record the initial child execution time */
        int init_exec_time = chld.exec_time;

        /* declare two time structs */
        long start, end;

        /* loop if there's time left */
        while( chld.exec_time > 0 ){
            // synchronize with scheduler
            char buf[8];
            /* use a blocking system call (read)
               to synchronize with the scheduler */
            read(chld.pipe_fd[0], buf, strlen("run"));
            
            /* if it is the first time being run */
            if( chld.exec_time == init_exec_time ){
                /* get the system time */
                start = syscall(SYS_GETTIME);
                /* print out child name and its pid */
                printf("%s %d\n", chld.name, getpid());
            }
            
            /* run the child process for a time unit */
            TIME_UNIT();
            /* decrement child's execution time */
            chld.exec_time--;
        }
        /* get system time, when the while loop finishes */
        end = syscall(SYS_GETTIME);
        /* use a self defined system call to write dmesg */
        syscall(SYS_PRINT, pid, start, end);
        /* child process exits */
        exit(0);
    }
    
    
    /* this part is what the parent process runs
       it runs before the child's 'read' call 
       gets executed */

    /* set it to very low priority */
    proc_kickout(chpid);
    /* assign chpid to cpu CHILD_CORE(3) */ 
    assign_core(chpid, CHILD_CORE);
    /* close pipe read descriptor of the parent
       'parent' here is not a typo */
    close( chld.pipe_fd[0] );

    /* return pid of the child */
    return chpid;
}

/* this function sets pid to the lowest prioity possible */
int proc_kickout(pid_t pid){
    Sched_pm sp;
    /* 0 is normal priority */
    sp.sched_priority = 0;

    /* set process to the lowest priority (IDLE) */
    if ( sched_setscheduler(pid, SCHED_IDLE, &sp) < 0 ){
        perror("error: sched_setscheduler");
        return -1;
    }

    return 0;
}

/* set pid to OTHER priority group */
int proc_resume(pid_t pid){
    Sched_pm sp;
    sp.sched_priority = 0;

    if ( sched_setscheduler(pid, SCHED_OTHER, &sp) < 0 ){
        perror("error: sched_setscheduler");
        return -1;
    }

    return 0;
}
