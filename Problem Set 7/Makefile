fifo_test.exe: fifo_test.o sem.o fifo.o
	gcc -o fifo_test.exe fifo_test.o sem.o fifo.o tas64.S

fifo_test.o: fifo_test.c fifo.h
	gcc -c fifo_test.c
sem.o: sem.c sem.h
	gcc -c sem.c

fifo.o: fifo.c fifo.h
	gcc -c fifo.c

debug:
	gcc -o fifo_debug.exe -g fifo_test.c fifo.c sem.c tas64.S

clean:
	rm *.exe *.o *.stackdump *~

TAS_test:
	gcc -o TAS_test TAS_test.c tas64.S