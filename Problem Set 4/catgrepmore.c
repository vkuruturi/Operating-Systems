//Venkat Kuruturi
//Pset 4
//catgrepmore

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

unsigned int files_processed = 0;
unsigned int bytes_processed = 0;
volatile sig_atomic_t nextfile = 0;
pid_t pid_grep, pid_more;

void sigpipe_handler(int sig){
	nextfile = 1;
	signal(sig, sigchild_handler);
}

void sigint_handler(int sig){
	fprintf(stderr,"Processed %d files.\n", files_processed);
	fprintf(stderr,"Processed %d bytes of data.\n", bytes_processed);
	exit(1);
}

int main(int argc, char* argv[]){
	signal(SIGPIPE, sigpipe_handler);
	signal(SIGINT, sigint_handler);
	
	if(argc < 3){
		fprintf(stderr,"Not enough arguments.\n");
		exit(1);
	}

	for(int i = 2; i <argc; i++){
		
	}
}