#ifndef SCHED_H
#define SCHED_H

#define SCHED_NPROC  256 
#define SCHED_READY 1
#define SCHED_RUNNING 2
#define SCHED_SLEEPING 3
#define SCHED_ZOMBIE 4

#define DEFAULT_PR 20

#include "jmpbuf-offsets64.h"


struct savectx 
{
	void *regs[JB_SIZE];
};

struct sched_proc 
{

	int pid;
	int ppid;			//Parent PID
	void *sp;			//Stack pointer
	int state;			//Task state
	int priority;		//Static priority, values from 0 to 39
	int accumulated;	//Total time process running since startup
	int CPUtime;		//
	int exitcode;		//exit value of process.  set by sched_exit
	int niceval;		//nice value, values from -20 to 19, default 0
	int vruntime; 		//dynamic priority
	struct savectx ctx; 
	struct sched_proc *parent;		//Parent process

};


struct sched_waitq 
{
	struct sched_proc *processes[SCHED_NPROC];
	int num_procs;
};

struct sched_proc *current;			//holds current process
struct sched_waitq *runq;			// run queue

int pid_count, tick_count;
int NEED_RESCHED;

void sched_init(void (*init_fn)());

int sched_fork();

void sched_exit(int code);

int sched_wait(int *code);

void sched_nice(int niceval);

int sched_getpid();

int sched_getppid();

int sched_gettick();

void sched_ps();

void sched_switch();

void sched_tick();

#endif