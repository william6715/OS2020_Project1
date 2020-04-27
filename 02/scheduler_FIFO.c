#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "scheduler.h"
#include "process_control.h"

//FIFO
int scheduler_FIFO(Process *proc, int N_procs){
	int cur = -1;
	int total_time=0;
	while(1){
		//the first process is 0,because it have sort 
		cur += 1;
        // break out the loop if we're finished with all the processes 
		if( cur >= N_procs ) break;
		//until it start to run
		while( proc[cur].ready_time > total_time ){
			TIME_UNIT();
			total_time += 1;
		}
		pid_t chpid = process_create(proc[cur]);
		process_resume( chpid );
		while( proc[cur].exec_time > 0 ){
			write(proc[cur].pipe_fd[1], "run", strlen("run"));
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
