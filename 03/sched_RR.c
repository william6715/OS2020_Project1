#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>
#include <string.h>

#include "scheduler.h"
#include "process_controler.h"

//RR_SLICE 500

int scheduler_RR(Process *proc, int N_procs){
	int finish = 0; 
	int time = 0; 
	while(1){
		int nt = 0; 
		//always run
		for(int i=0; i<N_procs; i++){
			if( time < proc[i].ready_time ){
				nt ++;
				continue;
			}
			else if( proc[i].exec_time <= 0 ){
				nt ++;
				continue;
			} 
			//if process can do will go here
			if( proc[i].pid > 0 ){
				process_resume( proc[i].pid );
			}
			else{ 
				proc[i].pid = process_create( proc[i] );
				write(proc[i].pipe_fd[1], "run", strlen("run"));
				process_resume( proc[i].pid );
			}
			// i can do, then do it, until it can't do
			int kt = RR_SLICE; 
			while( proc[i].exec_time > 0 && kt > 0){
				TIME_UNIT(); 
				proc[i].exec_time --;
				time ++;
				kt --;
			}
			if(proc[i].exec_time <= 0){
				int _return;
				waitpid(proc[i].pid, &_return, 0);
				if( !(WIFEXITED(_return)) ){
					perror("error: child process terminated inappropriately");
					return 1;
				}
				finish ++;
			}
			else process_kickout( proc[i].pid );	
		} 
		if( finish >= N_procs ) break;
		//all can't do, and at least one can do, but not come
		if( nt >= N_procs){ 			
			int next = find_next(proc, N_procs);
			while( proc[next].ready_time > time ){
				TIME_UNIT();
				time += 1;
			}
		}
	}

	return 0;
}
