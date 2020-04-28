#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/syscall.h>

#include "process_controler.h"

int find_shortest(Process *proc, int N_procs, int time){
	int shortest = -1;
	int min_time = INT_MAX;
	for (int i = 0; i < N_procs; i++){
		if (proc[i].ready_time <= time && proc[i].exec_time && proc[i].exec_time < min_time){
			excute_time = proc[i].exec_time;
			shortest = i;
		}
	}
	return shortest;
}

int scheduler_SJF(Process *proc, int N_procs){
	int finish = 0;
	int time = 0;
    /* keep looping if there are still unfinished processes */
	while (finish < N_procs){
        /* target = the index of the shortest job that is able to run 
           right now */
		int target = find_shortest(proc, N_procs, time);
		
        /* if such a target exists */
		if (target != -1){
            /* create process for the shortest job */
			pid_t chpid = process_create(proc[target]);
            /* raise its priority group */
			process_resume( chpid );
            /* set its process id */
			proc[target].pid = chpid;

            /* this while loop runs the child process
               until its execution time reaches 0 */
			while (proc[target].exec_time > 0){
				// tell child process to run 1 time unit
				char tmp[5] = "run";
				write(proc[target].pipe_fd[1], tmp, strlen(tmp));
				
				TIME_UNIT();
				time++;
				proc[target].exec_time--;
			}

            /* increment number of finished processes */
			finish++;			
			// wait child process
			int _return;
			waitpid(proc[target].pid, &_return, 0);
			
			if (WIFEXITED(_return) != 1){
				fprintf(stderr, "error: child process terminated inappropriately");
				return 1;
			}
		}

        /* control reaches this point if there isn't 
           any process able to run right now */
		else{
            /* run for a time unit */
			TIME_UNIT();
            /* increment time */
			time++;
		}
	}
	return 0;
}
