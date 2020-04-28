#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sched.h>

#include "scheduler.h"
#include "process_controler.h"


int main(){
	char sched[8];
	int N;
	scanf("%s", sched);
	scanf("%d", &N);
	Process* process = malloc(N * sizeof(Process));
	//it will be sort by ready time already
	for(int i = 0 ; i < N ; i++){
		//initial 
		scanf("%s %d %d", process[i].name, &(process[i].ready_time), &(process[i].exec_time));
		process[i].pid = -1; 
		if(pipe((process[i].pipe_fd)) < 0){
			perror("error: pipe can't set up");
			exit(1);
		}
	}

    // assign the main process to cpu 2 
	// my virtual box have 4 cpu
	// and put schedule process to high priority
	assign_core(getpid(), SCHED_CORE);
	process_highest(getpid());

	if(sched[0] == 'F') scheduler_FIFO(process, N);
	else if(sched[0] == 'R') scheduler_RR(process, N);
	else if(sched[0] == 'S') scheduler_SJF(process, N);
	else if(sched[0] == 'P') scheduler_PSJF(process, N);
	else printf("No Such scheduler method.\n");

	//make back to idle
	struct sched_param param;
    param.sched_priority = 0;
    if ( sched_setscheduler(getpid(), SCHED_OTHER, &param) < 0 ){
        perror("error: sched_setscheduler can't get back");
        return -1;
    }
	return 0;
}
