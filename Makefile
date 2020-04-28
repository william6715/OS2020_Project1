CFLAG = -fgnu89-inline

all:
	gcc $(CFLAG) main.c -c
	gcc $(CFLAG) sched/FIFO.c -c
	gcc $(CFLAG) sched/RR.c -c
	gcc $(CFLAG) sched/SJF.c -c
	gcc $(CFLAG) sched/PSJF.c -c
	gcc $(CFLAG) process_controler.c -c
	gcc main.o FIFO.o RR.o SJF.o PSJF.o process_controler.o -o 2020OS_Project_1

clean:
	rm -f *.o 2020OS_Project_1

