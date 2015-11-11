//Venkat Kuruturi
//PSet 7
//Test file for FIFO and semaphores

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

#include "sem.h"
#include "fifo.h"

#define N_PROC 64

int main(int argc, char **argv){
	/* For readers not to hang, readers*readBytes <= writers*writeBytes */
	int readers = 1; int writers = 6;
	int readBytes = 18; int writeBytes = 3;
	if((readers+writers)>N_PROC){fprintf(stderr, "Error - Too Many Processes Opened\n"); return -1;}

	struct fifo *f;
	f = mmap(NULL, sizeof *f, PROT_READ|PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS,-1,0);
	if(f==MAP_FAILED){ perror("Error Mapping Fifo to Memory"); return -1;}
	fifo_init(f);
	fprintf(stderr, "MMAP DONE\n");
	int i,p; int j = 0;
	unsigned long d;
	for(i=1; i<=readers; i++){
		p=fork();
		if(p==-1){
			perror("Error Forking a Reader");
			return -1;
		}
		else if(!p){
			for(j=0;j<readBytes;j++){
				p = getpid();
				d = fifo_rd(f);
				fprintf(stderr, "\t\t\tReader #%d = %lu\n",i,d);
			}
			return 0;
		}
	}
	for(i=1; i<=writers; i++){
		p=fork();
		if(p==-1){
			perror("Error Forking a Writer");
			return -1;
		}
		else if(!p){
			for(j=1;j<=writeBytes;j++){
				p = getpid();
				d = i*100+j;
				fifo_wr(f,d);
				fprintf(stderr, "Writer #%d = %lu\n",i,d);
			}
			return 0;
		}
	}

	int stat;
	for(i=0;i<readers+writers;i++){
		if(wait(&stat)==-1){ perror("Error Returning from Child Process"); return -1;}
	}
	return 0;
}