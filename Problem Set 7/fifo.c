//Venkat Kuruturi
//Pset 7
//fifo.c

#include "fifo.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

#include "sem.h"

void fifo_init(struct fifo *f){
	f->head = 0;
	f->tail = 0;

	sem_init(&(f->rd), 0);
	sem_init(&(f->wr), MYFIFO_BUFSIZ);
	sem_init(&(f->access), 1);
}

void fifo_wr(struct fifo *f, unsigned long d){
	sem_wait(&(f->wr));
	sem_wait(&(f->access));

	f->buf[f->head++] = d;
	f->head %= MYFIFO_BUFSIZ;

	sem_inc(&(f->rd));
    sem_inc(&(f->access));

}


unsigned long fifo_rd(struct fifo *f) {
    unsigned long rd;

    sem_wait(&(f->rd));
    sem_wait(&(f->access)); 
 
    rd = f->buf[f->tail++];
    f->tail %= MYFIFO_BUFSIZ;

    sem_inc(&(f->wr));
    sem_inc(&(f->access));

    return rd;
}