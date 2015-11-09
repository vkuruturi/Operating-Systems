//Venkat Kuruturi
//TAS_test.c

//if no arguments are provided, runs without mutex
//else mutex is used.
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

extern int tas(int *lock);
pid_t pid1 = 0;
pid_t pid2 = 0;
int tries = 0;

struct test{
	unsigned long long var;
	int lock;
};
struct test *t;

void sigint_handler(int sig){
	if(pid1 == 0 || pid2 ==0)
		exit(1);
	fprintf(stderr,"SIGINT received. Exiting.\n");
	fprintf(stderr,"Trial count: %d\n", tries);
	exit(1);
}
int main(int argc, char* argv[]){
	signal(SIGINT, sigint_handler);
	
	t = mmap(NULL, sizeof *t, PROT_READ | PROT_WRITE,
		MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	t->lock = 0;
	while(1){
		if((pid1 = fork()) < 0){
			perror("Fork failed");
			exit(1);
		}
		else if(pid1 == 0){
			if(argc == 1){
				t->var+= 10;
				exit(0);
			}
			else{
				while(tas(&t->lock));
				t->var+=10;
				t->lock = 0;
				exit(0);
			}
		}

		if((pid2 = fork()) < 0){
			perror("Fork failed");
			exit (1);
		}
		else if(pid2 == 0){
			if(argc == 1){
				t->var+= 20;
				exit(0);
			}
			else{
				while(tas(&t->lock));
				t->var+=20;
				t->lock = 0;
				exit(0);
			}
		}

		if(pid1 > 0 && pid2 > 0){
			int stat1, stat2;
			waitpid(pid1, &stat1, 0);
			waitpid(pid2, &stat2, 0);
			tries++;
			if(t->var == 30){
				pid1 = 0;
				pid2 = 0;
				t->var = 0;
			}
			else{
				fprintf(stderr, "Incorrect result observed. Value: %llu\n", t->var);
				fprintf(stderr, "Number of trials: %d\n",tries);
				exit(1);
			}

			if(tries > 100000){
				fprintf(stderr, "Tried concurrent operations 100 thousand times.  No inconsistencies found.  Exiting...\n");
				exit(0);
			}
		}
	}
}