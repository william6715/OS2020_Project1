#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "scheduler.h"
#include "process_controler.h"

//First-in first-out scheduler
int scheduler_FIFO(Process *proc, int N_procs){
	//initial
	int cur = -1;
	int time = 0;
	while(1){
		cur += 1;
		if( cur >= N_procs ) break;
		//until this process can run
		while( proc[cur].ready_time > time ){
			TIME_UNIT();
			time += 1;
		}
		pid_t chpid = process_create(proc[cur]);
		proc[cur].pid = chpid;
		process_resume( chpid );
		//run
		while( proc[cur].exec_time > 0 ){
			write(proc[cur].pipe_fd[1], "run", strlen("run"));
			TIME_UNIT();
			++time;
			proc[cur].exec_time -= 1;
		}
		//waitpid
		int _return;
		waitpid(chpid, &_return, 0);
		if( !(WIFEXITED(_return)) ){
			perror("error: child process terminated inappropriately");
			return 1;
		}
	}
	return 0;
}
