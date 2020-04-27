#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "scheduler.h"
#include "process_control.h"

/* First-in first-out scheduler */
int scheduler_FIFO(Process *proc, int N_procs){

    /* cur is the index of the process */
	int cur = -1;
	int total_time=0;

	while(1){

		cur += 1;
        /* break out of the loop if we're finished 
           with all the processes */
		if( cur >= N_procs ) break;

        /* keep running TIME_UNIT until the total time
           reaches the current process ready time */
		while( proc[cur].ready_time > total_time ){
			TIME_UNIT();
			// wait 1 time unit until proc is ready
			total_time += 1;
		}
        /* use proc_create to fork a child,
           after this call, the child should be blocked
           in the 'read' system call, also child is
           assigned to cpu 3, and its priority is set
           very low */
		pid_t chpid = proc_create(proc[cur]);
        /* set child process priority to group OTHER */
		proc_resume( chpid );
        /* keep looping if the execution time is positive */
		while( proc[cur].exec_time > 0 ){
			// tell process to run 1 time unit
			write(proc[cur].pipe_fd[1], "run", strlen("run"));
			// run 1 time unit itself
			TIME_UNIT();
			total_time += 1;
			proc[cur].exec_time -= 1;
		}

		// wait child process
		int re_status;
		waitpid(chpid, &re_status, 0);
		if( !(WIFEXITED(re_status)) ){
			perror("error: child process terminated inappropriately");
			return 1;
		}

	}

	return 0;
}
