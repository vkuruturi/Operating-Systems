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
	if (argc < 5){
		fprintf(stderr, "Usage: ./fifo_test.exe [# of writers] [#of bytes to write per writer] [#of readers] [#of bytes to read per reader]\n");
		exit(1);
	}

	int readers = atoi(argv[3]);
	int writers = atoi(argv[1]);
	int readBytes = atoi(argv[4]);
	int writeBytes = atoi(argv[2]);
	if((readers+writers)>N_PROC){
		fprintf(stderr, "Error: Too many processes requested.\n"); 
		exit(1);
	}

	struct fifo *f;
	f = mmap(NULL, sizeof *f, PROT_READ|PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS,-1,0);
	if(f==MAP_FAILED){
		perror("Error using mmap"); 
		exit(1);
	}
	fifo_init(f);
	int i,p; int j = 0;
	unsigned long d;

	for(i=1; i<=writers; i++){
		p=fork();
		if(p==-1){
			perror("Error using fork");
			exit(1);
		}
		else if(!p){
			for(j=1;j<=writeBytes;j++){
				d = i*100+j;
				fifo_wr(f,d);
				fprintf(stderr, "Writer #%d = %lu\n",i,d);
			}
			exit(0);
		}
	}

	for(i=1; i<=readers; i++){
		p=fork();
		if(p==-1){
			perror("Error Forking a Reader");
			exit(1);
		}
		else if(!p){
			for(j=0;j<readBytes;j++){
				d = fifo_rd(f);
				fprintf(stderr, "\t\t\tReader #%d = %lu\n",i,d);
			}
			exit(0);
		}
	}	


	int stat;
	for(i=0;i<readers+writers;i++){
		if(wait(&stat)==-1){
			perror("Error Returning from Child Process"); 
			exit(1);
		}
	}
	exit(0);
}