#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>

#define LOOPSIZE 500000
struct timespec start;
struct timespec end;
void caseA();
void caseB();
void caseC();

void f(){
	return;
}

void gettime(clockid_t clk_id, struct timespec* tp)
{
	if (clock_gettime(clk_id, tp) == -1)
	{
		perror("clock_gettime");
		exit(1);
	}
}

int printTime()
{
	if (end.tv_nsec < start.tv_nsec)
	{
		printf("%ld seconds ", end.tv_sec - start.tv_sec - 1);
		printf("and %ld nanoseconds\n", 1000000000 + end.tv_nsec - start.tv_nsec);
		printf("Average cost per iteration: %lld nanoseconds\n",  ((long long)(1000000000 +end.tv_nsec -start.tv_nsec) + (end.tv_sec - start.tv_sec - 1)*1000000000)/LOOPSIZE);
	}
	else
	{
		printf("%ld seconds ", end.tv_sec - start.tv_sec);
		printf("and %ld nanoseconds\n", end.tv_nsec - start.tv_nsec);
		printf("Average cost per iteration: %ld nanoseconds\n", (end.tv_nsec - start.tv_nsec)/LOOPSIZE);
	}
}

void caseA()
{
	int i = 0;

	gettime(CLOCK_REALTIME, &start);
	for (i = 0; i < LOOPSIZE; i++);
	gettime(CLOCK_REALTIME, &end);
	printf("One empty for loop of %d iterations costs ", LOOPSIZE);
	printTime();
}

void caseB()
{
	int i = 0;

	gettime(CLOCK_REALTIME, &start);
	
	for (i = 0; i < LOOPSIZE; i++)
		f();

	gettime(CLOCK_REALTIME, &end);
	printf("One user-mode function in a for loop of %d iterations costs ", LOOPSIZE);
	printTime();
}

void caseC()
{
	int i = 0;

	gettime(CLOCK_REALTIME, &start);

	for (i = 0; i < LOOPSIZE; i++)
		getuid();
	
	gettime(CLOCK_REALTIME, &end);
	printf("One system call in a for loop of %d iterations costs ", LOOPSIZE);
	printTime();
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		fprintf(stderr, "Wrong number of arguments.\n");
		exit(1);
	}
	switch (*(argv[1]))
	{
	case ('A'):
		caseA();
		break;
	case ('B'):
		caseB();
		break;
	case ('C'):
		caseC();
		break;
	default:
		fprintf(stderr, "Invalid argument.  Please select A, B or C.\n");
		exit(1);
		break;
	}
	return 0;
}

