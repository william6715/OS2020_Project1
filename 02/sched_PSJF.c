#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/syscall.h>

#include "scheduler.h"
#include "process_controler.h"


int find_shortest(Process *proc, int N_procs, int time){
	int shortest = -1, 
	int min_time = INT_MAX;
	for (int i = 0; i < N_procs; i++){
		if (proc[i].ready_time <= time && proc[i].exec_time && proc[i].exec_time < min_time){
			min_time = proc[i].exec_time;
			shortest = i;
		}
	}
	return shortest;
}

/* Preemptive shortest job first scheduler */
int scheduler_PSJF(Process *proc, int N_procs){
	int time = 0, last_turn = -1;;

    /* finish: number of processes finished */
	int finish = 0, started[N_procs];
    /* initialize started with 0s */
	memset(started, 0, sizeof(started));
    /* keep looping while there're still unfinished processes */
	while (finish < N_procs){
        /* target = index of the shortest job runnable  */
		int target = find_shortest(proc, N_procs, time);
		
        /* if such a job exists */
		if (target != -1){
            /* if target has not been created yet */
			if (started[target] == 0){
                /* create the process */
				pid_t chpid = process_create(proc[target]);
                /* raise its priority group */
				process_resume( chpid );

                /* set pid */
				proc[target].pid = chpid;
                /* set the index to be 'started' */
				started[target] = 1;
			}
            /* target has been created in a previous iteration */
			else {				
                /* simply raise its priority group */
				process_resume( proc[target].pid );
			}
            /* record the index of the process to be run 
               variable 'last_turn' is only used for logging information*/
			last_turn = target;

			// tell child process to run 1 time unit
			char tmp[5] = "run";
            /* send run message */
			write(proc[target].pipe_fd[1], tmp, strlen(tmp));
            /* also run a time unit */
			TIME_UNIT();
            /* increment time */
			time++;

            /* decrement its execution time */
			proc[target].exec_time--;		
            /* lower its priority group */
			process_kickout( proc[target].pid );
			
            /* if the process finishes execution */
			if (proc[target].exec_time == 0){		
				// wait child process
				int _return;
				waitpid(proc[target].pid, &_return, 0);
				
				if (WIFEXITED(_return) != 1){
					fprintf(stderr, "error: child process terminated inappropriately");
					return 1;
				}

                /* increment number of finished processes */
				finish++;
			}
		}		
        /* if we can't find a process that's able to run right now */
		else{
            /* run for a time unit */
			TIME_UNIT();
            /* increment time */
			time++;
		}
	}
	return 0;
}
