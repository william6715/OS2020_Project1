#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/syscall.h>

#include "process_control.h"

/*  */
int find_shortest(Process *proc, int N_procs, int time){
	int shortest = -1, excute_time = INT_MAX;

    /* loop through all processes */
	for (int i = 0; i < N_procs; i++){
        /* if process is ready && process execution time is not null && execution time is smaller than 
           the previous least, update the previous least */
		if (proc[i].ready_time <= time && proc[i].exec_time && proc[i].exec_time < excute_time){
			excute_time = proc[i].exec_time;
            /* update the index of the shortest process */
			shortest = i;
		}
	}
    /* return index of the shortest process */
	return shortest;
}

/* Shortest job first scheduler */
int scheduler_SJF(Process *proc, int N_procs){
    /* number of finished processes */
	int finish = 0;
    /* current time */
	int time = 0;

/* prints process excution details if we define PRINT_LOG */
#ifdef PRINT_LOG
	FILE *fp = fopen("./scheduler_log/SJF_5.out", "wb");
	char mesg[256] = "";
#endif

    /* keep looping if there are still unfinished processes */
	while (finish < N_procs){
        /* target = the index of the shortest job that is able to run 
           right now */
		int target = find_shortest(proc, N_procs, time);
		
        /* if such a target exists */
		if (target != -1){
/* second part of the PRINT_LOG */
#ifdef PRINT_LOG
			sprintf(mesg, "process %s, start at %d\n", proc[target].name, time);
			fprintf(fp, "%s", mesg);
			fflush(fp);
#endif

            /* create process for the shortest job */
			pid_t chpid = proc_create(proc[target]);
            /* raise its priority group */
			proc_resume( chpid );
            /* set its process id */
			proc[target].pid = chpid;

            /* this while loop runs the child process
               until its execution time reaches 0 */
			while (proc[target].exec_time > 0){
				// tell child process to run 1 time unit
				char tmp[5] = "run";
				write(proc[target].pipe_fd[1], tmp, strlen(tmp));
				
				TIME_UNIT();
				time++;
				proc[target].exec_time--;
			}

            /* increment number of finished processes */
			finish++;			
			// wait child process
			int _return;
			waitpid(proc[target].pid, &_return, 0);
			
			if (WIFEXITED(_return) != 1){
				fprintf(stderr, "error: child process terminated inappropriately");
				return 1;
			}
#ifdef PRINT_LOG
			sprintf(mesg, "process %s, end at %d\n", proc[target].name, time);
			fprintf(fp, "%s", mesg);
			fflush(fp);
#endif
		}

        /* control reaches this point if there isn't 
           any process able to run right now */
		else{
            /* run for a time unit */
			TIME_UNIT();
            /* increment time */
			time++;
		}
	}

/* close file pointer if we used PRINT_LOG */
#ifdef PRINT_LOG
	fclose(fp);
#endif

	return 0;
}
