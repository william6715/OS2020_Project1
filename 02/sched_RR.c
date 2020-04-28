#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>
#include <string.h>

#include "scheduler.h"
#include "process_controler.h"

int scheduler_RR(Process *proc, int N_procs){
	int finish = 0; 
	int time = 0; 

	while(1){

		int nt = 0; // number of processes not allowed to be executed
		int next_ex_t = INT_MAX; //the closest ready time from current time (used when nt == N_procs)

        /* loop through each process */
		for(int i=0; i<N_procs; i++){

            /* if the current time is less than the 
               current process's ready time */
			if( time < proc[i].ready_time ){
                /* update the closest ready time if necessary */
				if( proc[i].ready_time < next_ex_t ) next_ex_t = proc[i].ready_time;
                /* also increment the number of processes not allowed to be executed */
				nt ++;
                /* go on to the next process */
				continue;
            /* if this process already finished its execution */
			}else if( proc[i].exec_time <= 0 ){
                /* also increment */
				nt ++;
                /* go on to the next process */
				continue;
			} 

            /* reaches here if the process in question is ready
               to run and does need more time to run */
            /* if the process has been created */
			if( proc[i].pid > 0 ){
                /* resume the process (raise priority group to OTHER) */
				process_resume( proc[i].pid );
			}else{ // if process hasn't been created
                /* create the process */
				proc[i].pid = process_create( proc[i] );
                /* raise the priority group */
				process_resume( proc[i].pid );
			}

			// run an RR round, originally set to 500 time units
			int kt = RR_SLICE; //time quantum for RR
			while( proc[i].exec_time > 0 && kt > 0){
				write(proc[i].pipe_fd[1], "run", strlen("run")); // tell process to run 1 time unit
				TIME_UNIT(); // run 1 time unit itself
                /* decrement time slice, process execution time 
                   and increment current time */
				kt --;
				proc[i].exec_time --;
				time ++;
			}

			// if process finished
            /* receive its exit status */
			if(proc[i].exec_time <= 0){
				int re_status;
				waitpid(proc[i].pid, &re_status, 0);
				if( !(WIFEXITED(re_status)) ){
					perror("error: child process terminated inappropriately");
					return 1;
				}
                /* increment number of process finished */
				finish ++;
            /* if the process have not yet finished (time slice used up) */
			}else{
                /* lower its priority */
				process_kickout( proc[i].pid );				
			}

		} /* for loop ends here */

        /* number of finished process >= number of processes */
		if( finish >= N_procs ) break;

        /* number of processes that can not run >= number of processes */
		if( nt >= N_procs){ // run itself when not finished and no process can be executed
            /* loop until a process is ready */
			while( time < next_ex_t ){ //until at least a process is ready
				TIME_UNIT();
				time ++;
			}
		}
	}

	return 0;
}
