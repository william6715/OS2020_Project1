#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/syscall.h>

#include "../scheduler.h"
#include "../process_controler.h"


int find_next_without_target(Process *proc, int N_procs, int* started,int time){
	int next = -1;
	int nexttime = INT_MAX;
	for (int i = 0; i < N_procs; i++){
		if (proc[i].exec_time && (proc[i].ready_time < nexttime) && (started[i] == 0) && proc[i].ready_time>time){
			nexttime = proc[i].ready_time;
			next = i;
		}
	}
	return next;
}

/* Preemptive shortest job first scheduler */
int scheduler_PSJF(Process *proc, int N_procs){
	int finish = 0;
	int time = 0;
	int started[N_procs];
    for(int i = 0; i<N_procs; ++i) started[i] = 0;
	while (finish < N_procs){
		int target = find_shortest(proc, N_procs, time);
		if (target != -1){
			if (started[target] == 0){//first time to it
				pid_t chpid = process_create(proc[target]);
				proc[target].pid = chpid;
				process_resume( chpid );
				started[target] = 1;
			}
			else {//resume to it				
				process_resume( proc[target].pid );
			}
			//find next start time
			int next = find_next_without_target(proc, N_procs, started,time);
			//do target until next process ready
			if(next!=-1){
				while(proc[next].ready_time > time){
					if(proc[target].exec_time <= 0)break;
					write(proc[target].pipe_fd[1], "run", strlen("run"));
					TIME_UNIT();
					++time;
					proc[target].exec_time--;		
				}
			}
			else{
				while( proc[target].exec_time > 0 ){
					write(proc[target].pipe_fd[1], "run", strlen("run"));
					TIME_UNIT();
					++time;
					proc[target].exec_time --;
				}
			}
			if (proc[target].exec_time <= 0){		
				int _return;
				waitpid(proc[target].pid, &_return, 0);
				if (WIFEXITED(_return) != 1){
					fprintf(stderr, "error: child process terminated inappropriately");
					return 1;
				}
				finish++;
			}
			else{
				process_kickout( proc[target].pid );
			}
		}		
		else{//mean the next is not ok
			//find the next process ok
			int next = find_next(proc, N_procs);
			while( proc[next].ready_time > time ){
				TIME_UNIT();
				time += 1;
			}
		}
	}
	return 0;
}
