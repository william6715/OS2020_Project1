CFLAG = -fgnu89-inline

all:
	gcc $(CFLAG) main.c -c
	gcc $(CFLAG) sched_FIFO.c -c
	gcc $(CFLAG) sched_RR.c -c
	gcc $(CFLAG) sched_SJF.c -c
	gcc $(CFLAG) sched_PSJF.c -c
	gcc $(CFLAG) process_controler.c -c
	gcc main.o sched_FIFO.o sched_RR.o sched_SJF.o sched_PSJF.o process_controler.o -o 2020OS_Project_1

clean:
	rm -f *.o 2020OS_Project_1

