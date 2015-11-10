//Venkat Kuruturi
//Pset 7
//Semaphore implementation

#include "sem.h"
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>

int tas(volatile char *lock);
void sigusr1_handler(int sig){
	return;
}

void sem_init(struct sem *s, int count){
	s->count = count;
	s->lock  = 0;

	for(int i = 0; i < N_PROC; i++){
		s->proc_status[i] = 0;
		s->procID[i] = 0;
	}
}

int sem_try(struct sem *s){
	while(tas(s->lock));
	if(s->count > 0){
		s->count--;
		s->lock = 0;
		return 1;
	}
	s->lock - 0;
	return 0;
}

void sem_wait(struct sem *s){
	s->procID[proc_num] = getpid();
	while(1){
		while(tas(s->lock));
		signal(SIGUSR1, sigusr1_handler);
		sigset_t mask, old_mask;
		sigfillset(&mask);
		sigdelset(&mask, SIGINT);
		sigdelset(&mask, SIGUSR1);
		sigprocmask(SIG_BLOCK, &mask, &oldmask);

		if(s->count > 0){
			s->count--;
			s->proc_status[proc_num] = 0;
			s->lock = 0;
			return;
		}
		else{
			s->lock = 0;
			s->proc_status[proc_num] = 1;
			sigsuspend(&mask);
		}

		sigprocmask(SIG_UNBLOCK, &mask, NULL);
	}
}

void sem_inc(struct sem *s){

	while(tas(s->lock));
	s->count++;

	for(int i = 0; i < N_PROC; i++){
		if(s->proc_status[i]){
			kill(s->procid[i], SIGUSR1);
		}
	}

	s->lock = 0;
}