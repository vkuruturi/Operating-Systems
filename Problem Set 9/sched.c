#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <limits.h>
#include "sched.h"
#include "adjstack.c"

#define STACK_SIZE 65536


void sched_init(void (*init_fn)()){
	NEED_RESCHED = 0;
	pid_count = 0;
	tick_count = 0;


	signal(SIGVTALRM, sched_tick);
	signal(SIGABRT, sched_ps);

	struct timeval time;
	time.tv_sec = 0;
	time.tv_usec = 500000;		//100000 microseconds = 100 milliseconds

	struct itimerval timer;
	timer.it_interval = time;
	timer.it_value = time;

	setitimer(ITIMER_VIRTUAL, &timer, NULL);


	if(!(runq = malloc(sizeof(struct sched_waitq)))){
		perror("Error mallocing running queue.\n");
		exit(1);
	}

	if(!(current = malloc(sizeof(struct sched_proc)))){
		perror("Error mallocing running queue.\n");
		exit(1);
	}

	void *new_sp;
	if ((new_sp=mmap(0,STACK_SIZE,PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS,0,0))==MAP_FAILED){
		perror("Error: mmap for initial stack failed.\n");
		exit(1);
	}

	int init_pid = get_new_pid();
	if (init_pid < 0){
		perror("Error allocating initial PID.\n");
		exit(1);
	}

	//Create initial process
	struct sched_proc *init;
	if(!(init = malloc(sizeof(struct sched_proc)))){
		perror("Error mallocing initial process.\n");
		exit(1);
	}

	init->pid =  init_pid;
	init->ppid = init_pid;
	init->sp = new_sp;					
	init->state = SCHED_RUNNING;			
	init->priority = DEFAULT_PR;
	init->accumulated = 0;
	init->CPUtime = 0;
	init->exitcode = 0;
	init->parent = init;
	init->niceval = 0;
	init->vruntime = 10;

	runq->processes[0] = init;
	runq->num_procs = 1;
	current = init;
	//pid_count++;

	struct savectx currentctx;
	 
	currentctx.regs[JB_BP] = new_sp + STACK_SIZE;
	currentctx.regs[JB_SP] = new_sp + STACK_SIZE;
	currentctx.regs[JB_PC] = init_fn;		//return addr in edx

	restorectx(&currentctx, 0);

	init->state = SCHED_SLEEPING;
	//sched_ps();
}

pid_t get_new_pid(){
	int i;
	for(i = 0; i < SCHED_NPROC; i++){
		if(!(runq->processes[i])){
			pid_count++;
			return i+1;
		}
	}
}

int sched_fork(){
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGVTALRM);
	sigprocmask(SIG_BLOCK, &set, NULL);
	fprintf(stderr, "in fork\n");
	struct sched_proc *child;
	if(!(child = malloc(sizeof(struct sched_proc)))){
		fprintf(stderr, "error malloc-ing child process\n");
		return -1;
	}
	int pid_child = get_new_pid();
	fprintf(stderr, "pid:%d\n",pid_child);
	if(pid_child<0){
		fprintf(stderr, "unable to allocate pid to child\n");
		return -1;
	}
	void *new_sp;
	if((new_sp = mmap(0,STACK_SIZE, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, 0 , 0)) == MAP_FAILED){
		perror("error mmapping child stack\n");
		return -1;
	}
	memcpy(new_sp, current->sp, STACK_SIZE);

	child->pid = pid_child;
	child->ppid = current->pid;
	child->sp = new_sp;
	child->state = SCHED_READY;
	child->priority = DEFAULT_PR;
	child->accumulated = current->accumulated;
	child->CPUtime = current->CPUtime;
	child->exitcode = 0;
	child->parent = current;
	child->niceval = 0;
	child->vruntime = current->vruntime;
	runq->processes[pid_child-1] = child;
	runq->num_procs++;
	//sched_ps();
	NEED_RESCHED = 1;
	int return_val = pid_child;
	if(!savectx(&child->ctx)){
		unsigned long long adj = child->sp - current->sp;
		adjstack(new_sp, new_sp+STACK_SIZE, adj);
		child->ctx.regs[JB_BP] += adj;
		child->ctx.regs[JB_SP] += adj;

	}
	else {	//in child
		fprintf(stderr, "inchild\n");
		//sigprocmask(SIG_UNBLOCK, &set, NULL);
		return_val = 0;
	}

	sigprocmask(SIG_UNBLOCK, &set, NULL);
	return return_val;
}

void sched_exit(int code){
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGVTALRM);
	sigprocmask(SIG_BLOCK, &set, NULL);
	current->state = SCHED_ZOMBIE;
	NEED_RESCHED = 1;
	current->exitcode = code;
	runq->num_procs--;

	if(current->parent->state ==SCHED_SLEEPING)
		current->parent->state = SCHED_READY;

	sigprocmask(SIG_UNBLOCK, &set, NULL);
};

int sched_wait(int *code){
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGVTALRM);
	sigprocmask(SIG_BLOCK, &set, NULL);

	int i;
	int flag = 0;		//flag to show if a child process exists
	fprintf(stderr, "in wait\n");
	for(i = 0; i < SCHED_NPROC; i++)
		if((runq->processes[i]) && (runq->processes[i]->ppid == current->pid))
			flag = 1;

	if(flag == 0){
		sigprocmask(SIG_UNBLOCK, &set, NULL);
		return -1;
	}
	fprintf(stderr, "child exists\n");
	flag = 0;			//now used to show presence of zombie that belongs to current
	while(1){
		for(i = 0; i < SCHED_NPROC; i ++){
			if((runq->processes[i]) && (runq->processes[i]->state== SCHED_ZOMBIE))
				if(runq->processes[i]->ppid == current->pid){
					flag = 1;
					break;
				}
		}

		if(flag == 1){
			break;
		}

		current->state = SCHED_SLEEPING;
		NEED_RESCHED = 1;
		sigprocmask(SIG_UNBLOCK, &set, NULL);
		sched_switch();
	}

	*code = runq->processes[i]->exitcode;
	free(runq->processes[i]);
	runq->processes[i] = NULL;
	sigprocmask(SIG_UNBLOCK, &set, NULL);
	return 0;
}

void sched_nice(int nice){
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGVTALRM);
	sigprocmask(SIG_BLOCK, &set, NULL);

	if(nice > 19)
		nice = 19;
	else if (nice < -20)
		nice = 20;
	current->niceval = nice;
	sigprocmask(SIG_UNBLOCK, &set, NULL);
}

int sched_getpid(){
	return current->pid;
}

int sched_getppid(){
	return current->ppid;
}

int sched_gettick(){
	return tick_count;
}


void sched_ps() {
	//fprintf(stderr, "%d\n", tick_count);
	printf("PID    PPID    STATE   STACK    NICE    DYNAMIC  TIME \n");
	int i;
	for (i = 0; i < SCHED_NPROC; i++) {
		if (runq->processes[i]){
			printf("%d\t",runq->processes[i]->pid);
			printf("%d\t",runq->processes[i]->ppid);
			printf("%d\t", runq->processes[i]->state);
			printf("%x\t",runq->processes[i]->sp);
			printf("%d\t",runq->processes[i]->niceval);
			printf("%d\t",runq->processes[i]->priority);
			printf("%d\n",runq->processes[i]->accumulated);
		}
	}
}

void update_priorities(){
	int i;
	for(i = 0; i < SCHED_NPROC;i++){
		if(runq->processes[i]){
			int priority = 20 - runq->processes[i]->niceval;
            priority -= (runq->processes[i]->accumulated/(20 - runq->processes[i]->niceval));
            runq->processes[i]->priority = priority;			
		}
	}
}

void sched_switch(){
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGVTALRM);
	sigprocmask(SIG_BLOCK, &set, NULL);
	fprintf(stderr,"in schedswitch\n");
	update_priorities();
	if(current->state == SCHED_RUNNING){
		NEED_RESCHED = 0;
		current->state = SCHED_READY;
	}
	
	int i = 0;
	int best_proc = 0;
	int best_priority = INT_MIN;
	fprintf(stderr, "%d    %d\n",i,SCHED_NPROC);
	
	for(i = 0; i < SCHED_NPROC; i++){
		//fprintf(stderr, "IN FOR LOOP\n");
		if((runq->processes[i] != NULL)){
			fprintf(stderr,"valid process, state:%d\n", runq->processes[i]->state);
			if((runq->processes[i]->state == SCHED_READY)) {
				fprintf(stderr, "process priority: %d  best: %d\n", runq->processes[i]->priority,best_priority);
				if((runq->processes[i]->priority) > best_priority){
					best_proc = i;
					best_priority = runq->processes[i]->priority;
				}
			}
		}
	}
	fprintf(stderr, "best process: %d , best priority: %d\n",best_proc,best_priority);
	
	fprintf(stderr, "Switched to pid %d\n", current->pid);
	sched_ps();

	if((runq->processes[best_proc]->pid == current->pid) && (current->state == SCHED_READY)){
		current->CPUtime = 0;
		current->state = SCHED_RUNNING;
		return;
	}
	if(savectx(&(current->ctx)) == 0){
		current->CPUtime = 0;
		current = runq->processes[best_proc];
		restorectx(&(current->ctx),1);
	}


	sigprocmask(SIG_UNBLOCK, &set, NULL);
}

void sched_tick(){
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGVTALRM);
	sigprocmask(SIG_BLOCK, &set, NULL);
	fprintf(stderr, "in schedtick\n");
	tick_count++;
	current->accumulated++;
	current->CPUtime++;
	current->vruntime = 100+ 2 * (current->niceval);	

	sigprocmask(SIG_UNBLOCK, &set, NULL);

	sched_switch();
}