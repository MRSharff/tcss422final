all: CPU2.o fifo_queue.o pcb.o mythread.o
	gcc CPU2.o fifo_queue.o pcb.o mythread.o -o z
CPU2.o: CPU2.c
	gcc -c CPU2.c
fifo_queue.o: fifo_queue.c fifo_queue.h
	gcc -c fifo_queue.c
pcb.o: pcb.c pcb.h
	gcc -c pcb.c
mutex.o: mutex.c mutex.h
	gcc -c mutex.c
cond_var_type.o: cond_var_type.c cond_var_type.h
	gcc -c cond_var_type.c