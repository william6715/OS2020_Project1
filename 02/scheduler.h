#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include "process_controler.h"
#define RR_SLICE 500

#define SCHED_CORE 2
#define CHILD_CORE 3

int scheduler_FIFO(Process *proc, int N_procs);
int scheduler_RR(Process *proc, int N_procs);
int scheduler_SJF(Process *proc, int N_procs);
int scheduler_PSJF(Process *proc, int N_procs);

int find_shortest(Process *proc, int N_procs, int time);

#endif
