all: operating_system.o fifo_queue.o pcb.o cond_var_type.o errors.o priority_queue.o mutex.o
	gcc operating_system.o fifo_queue.o pcb.o cond_var_type.o errors.o priority_queue.o mutex.o -o z
operating_system.o: operating_system.c operating_system.h
	gcc -c operating_system.c
cond_var_type.o: cond_var_type.c cond_var_type.h
	gcc -c cond_var_type.c
mutex.o: mutex.c mutex.h
	gcc -c mutex.c
errors.o: errors.c errors.h
	gcc -c errors.c
priority_queue.o: priority_queue.c priority_queue.h
	gcc -c priority_queue.c
fifo_queue.o: fifo_queue.c fifo_queue.h
	gcc -c fifo_queue.c
pcb.o: pcb.c pcb.h
	gcc -c pcb.c
