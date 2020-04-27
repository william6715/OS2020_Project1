#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/syscall.h>

#include "process_control.h"

/* preemptive version of the find shortest function
   it's actually the same function, but it is used
   differently than in the SJF scheduler */
int preemptive_find_shortest(Process *proc, int N_procs, int time){
	int shortest = -1, excute_time = INT_MAX;

    /* loop through each process */
	for (int i = 0; i < N_procs; i++){
		if (proc[i].ready_time <= time && proc[i].exec_time && proc[i].exec_time < excute_time){
			excute_time = proc[i].exec_time;
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

#ifdef PRINT_LOG
	FILE *fp = fopen("./scheduler_log/PSJF_5.out", "wb");
	char mesg[256] = "";
#endif
	
    /* keep looping while there're still unfinished processes */
	while (finish < N_procs){
        /* target = index of the shortest job runnable  */
		int target = preemptive_find_shortest(proc, N_procs, time);
		
        /* if such a job exists */
		if (target != -1){
            /* if target has not been created yet */
			if (started[target] == 0){
#ifdef PRINT_LOG
				sprintf(mesg, "process %s, start at %d\n", proc[target].name, time);
				fprintf(fp, "%s", mesg);
				fflush(fp);
#endif
                /* create the process */
				pid_t chpid = proc_create(proc[target]);
                /* raise its priority group */
				proc_resume( chpid );

                /* set pid */
				proc[target].pid = chpid;
                /* set the index to be 'started' */
				started[target] = 1;
			}
            /* target has been created in a previous iteration */
			else {
#ifdef PRINT_LOG
				if (last_turn != target){										
					sprintf(mesg, "process %s, resume at %d\n", proc[target].name, time);
					fprintf(fp, "%s", mesg);
					fflush(fp);
				}
#endif
				
                /* simply raise its priority group */
				proc_resume( proc[target].pid );
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
			proc_kickout( proc[target].pid );
			
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
#ifdef PRINT_LOG
				sprintf(mesg, "process %s, end at %d\n", proc[target].name, time);
				fprintf(fp, "%s", mesg);
				fflush(fp);
#endif
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

#ifdef PRINT_LOG
	fclose(fp);
#endif

	return 0;
}
