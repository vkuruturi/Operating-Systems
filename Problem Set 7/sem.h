//Venkat Kuruturi
//Pset 7
//Header file for semaphores
#ifndef SEM_H_
#define SEM_H_

#define N_PROC 64
int proc_num;

/* 
 * Struct defintion for a sem structure
 */
struct sem {
	char lock;
    int count;
    int proc_status[N_PROC];
    int procID[N_PROC];
};

/*
 * Initialize the semaphore *s with the initial count. Initialize
 * any underlying data structures.  sem_init should only be called
 * once in the program.  If called after the semaphore has been
 * used, results are unpredictable.
 */
void sem_init(struct sem *s, int count);

/*
 * Attempt to perform the "P" operation (atomically decrement
 * the semaphore).  If this operation would block, return 0,
 * otherwise return 1.
 */
int sem_try(struct sem *s);

/*
 * Perform the P operation, blocking until successful.  Blocking
 * should be accomplished by noting within the *s that the current
 * virtual processor needs to be woken up, and then sleeping  using
 * the sigsuspend system call until SIGUSR1 is received.  Assume
 * that the extern int variable my_procnum exists and contains
 * the virtual processor id of the caller.  The implementation by
 * which you keep track of waiting processors is up to you.
 */
void sem_wait(struct sem *s);

/*
 * Perform the V operation.  If any other processors were sleeping
 * on this semaphore, wake them by sending a SIGUSR1 to their
 * process id (which is not the same as the virtual processor number).
 */
void sem_inc(struct sem *s);

#endif