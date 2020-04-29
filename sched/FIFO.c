#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "../scheduler.h"
#include "../process_controler.h"

//First-in first-out scheduler
int scheduler_FIFO(Process *proc, int N_procs){
	//initial
	int finish = 0;
	int time = 0;
	while(finish < N_procs){
		//stall until this process can run
		while( proc[finish].ready_time > time ){
			TIME_UNIT();
			time += 1;
		}
		pid_t chpid = process_create(proc[finish]);
		proc[finish].pid = chpid;
		process_resume( chpid );
		//run
		while( proc[finish].exec_time > 0 ){
			write(proc[finish].pipe_fd[1], "run", strlen("run"));
			TIME_UNIT();
			++time;
			proc[finish].exec_time -= 1;
		}
		//waitpid
		int _return;
		waitpid(chpid, &_return, 0);
		if( !(WIFEXITED(_return)) ){
			perror("error: child process terminated inappropriately");
			return 1;
		}
		finish++;
	}
	return 0;
}
