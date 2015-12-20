schedtest.exe: schedtest.o sched.o savectx64.S
	gcc -g -o schedtest.exe schedtest.c sched.c savectx64.S

schedtest.o: schedtest.c
	gcc -c schedtest.c

sched.o: sched.c sched.h jmpbuf-offsets64.h
	gcc -c sched.c

debug: 
	gcc -g -o schedDebug.exe schedtest.c  sched.c adjstack.c savectx64.S

clean:
	rm -f *.exe *.o *.stackdump *~

backup:
	test -d backups || mkdir backups
	cp *.c backups
	cp *.h backups
	cp Makefile backups
