CC = gcc
CFLAGS = -o
LDFLAGS = -m elf_x86_64 -o
ASFLAGS = --64 -o
p1:	p1.c
	$(CC) $(CFLAGS) p1 p1.c 

p2: p2.S
	as $(ASFLAGS) p2.o p2.S
	ld $(LDFLAGS) p2 p2.o
	rm p2.o

p3: p3.S
	as $(ASFLAGS) p3.o p3.S
	ld $(LDFLAGS) p3 p3.o
	rm p3.o

p4: p4.S
	as $(ASFLAGS) p4.o p4.S
	ld $(LDFLAGS) p4 p4.o
	rm p4.o

p5: p5.c
	$(CC) $(CFLAGS) p5 p5.c