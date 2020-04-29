#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "../scheduler.h"
#include "../process_controler.h"

//RR_SLICE 500
#define QUEUE_SIZE 256
int queue[QUEUE_SIZE];
int head = 0;
int tail = 0;
//0 mean not in yet
int first_time_in_queue[30];
//because N<=20, so will not full
void putin(int target){
	queue[tail] = target;
	first_time_in_queue[target] = 1;
	++tail;
	if(tail >= QUEUE_SIZE) tail -= QUEUE_SIZE;
}
int getnext(){
	int target = -1;
	if(tail != head){
		target = queue[head];
		head++;
		if(head >= QUEUE_SIZE) head -= QUEUE_SIZE;
		return target;
	}
	else return target;
}



int scheduler_RR(Process *proc, int N_procs){
	int finish = 0; 
	int time = 0; 
	for(int i = 0;i<30;i++)first_time_in_queue[i] = 0;
	while(finish < N_procs){
		//first check if queue have people
		int target = getnext();
		if(target != -1){//mean have at leat one need run
			if( proc[target].pid >= 0 ){
				process_resume( proc[target].pid );
			}
			else{ 
				proc[target].pid = process_create( proc[target] );
				process_resume( proc[target].pid );
			}
			// make target do, until it can't do
			int cycle = RR_SLICE; 
			while( proc[target].exec_time > 0 && cycle > 0){
				write(proc[target].pipe_fd[1], "run", strlen("run"));
				TIME_UNIT(); 
				proc[target].exec_time --;
				++time;
				--cycle;
			}
			//find any one need put into queue
			if(first_time_in_queue[N_procs-1]==0){
				for(int i=target+1;i<N_procs;++i){
					if(proc[i].ready_time < time && first_time_in_queue[i]==0 )putin(i);
				}
			}
			//exec ok
			if(proc[target].exec_time <= 0){
				int _return;
				waitpid(proc[target].pid, &_return, 0);
				if( !(WIFEXITED(_return)) ){
					perror("error: child process terminated inappropriately");
					return 1;
				}
				finish ++;
			}
			else{
				process_kickout( proc[target].pid );	
				putin(target);
			}
		}
		else{//queue empty, and at least one can do, but not come, finish< N_procs		
			int next = find_next(proc, N_procs);
			while( proc[next].ready_time > time ){
				TIME_UNIT();
				time += 1;
			}
			for(int i=0;i<N_procs;++i){
				if(proc[i].ready_time == proc[next].ready_time)putin(i);
			}
		}
	}

	return 0;
}
