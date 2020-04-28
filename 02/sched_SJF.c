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
			min_time = proc[i].exec_time;
			shortest = i;
		}
	}
	return shortest;
}

int scheduler_SJF(Process *proc, int N_procs){
	int finish = 0;
	int time = 0;
	while (finish < N_procs){
		int target = find_shortest(proc, N_procs, time);
		//mean still have process need run
		if (target != -1){
			pid_t chpid = process_create(proc[target]);
			proc[target].pid = chpid;
			process_resume( chpid );
			//run
			while (proc[target].exec_time > 0){
				write(proc[target].pipe_fd[1], "run", strlen("run"));
				TIME_UNIT();
				++time;
				proc[target].exec_time--;
			}
			finish++;			
			int _return;
			waitpid(proc[target].pid, &_return, 0);
			if (WIFEXITED(_return) != 1){
				fprintf(stderr, "error: child process terminated inappropriately");
				return 1;
			}
		}
		else{
			//find the next process ok
			TIME_UNIT();
			time++;
			//while( proc[cur].ready_time > time ){
			//	TIME_UNIT();
			//	time += 1;
			//}
		}
	}
	return 0;
}
