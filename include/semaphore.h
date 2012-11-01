#ifndef __SEMAPHORE_H
#define __SEMAPHORE_H

#include <io.h>
#include <sched.h>

typedef struct {
	u32 counter;
} semaphore_t;

typedef struct {
	semaphore_t sem;
} mutex_t;

#define SEMAPHORE_INIT(v)	{ (v) }

static inline void
semaphore_init(semaphore_t *sem, u32 value)
{
	sem->counter = value;
}

/* returns 1 if managed to decreased the counter
 * returns 0 if not */
static inline int
semaphore_trywait(semaphore_t *sem)
{
	u32 tmp;

	asm volatile (
		"ldrex v1, [%1]		\n\t"
		"teq v1, #0		\n\t"
		"moveq %0, #1		\n\t"
		"subne v1, v1, #1	\n\t"
		"strexne %0, v1, [%1]	\n\t"
		: "=&r" (tmp)
		: "r" (&sem->counter)
		: "v1", "memory", "cc"
	);

	if (tmp == 0) {
		dmb();
		return 1;
	} else
		return 0;
}

/* blocks until it manage to decrease the counter */
static inline void
semaphore_wait(semaphore_t *sem)
{
	while (!semaphore_trywait(sem))
		suspend_task((u32)sem); /* sleep */
}

/* increases the counter and resumes other tasks */
static inline void
semaphore_done(semaphore_t *sem)
{
	dmb();
	asm volatile (
		"1:			\n\t"
		"ldrex v1, [%0]		\n\t"
		"add v1, v1, #1		\n\t"
		"strex v2, v1, [%0]	\n\t"
		"teq v2, #0		\n\t"
		"bne 1b			\n\t"
		:
		: "r" (&sem->counter)
		: "v1", "v2", "memory", "cc"
	);
	dmb();
	/* resume tasks that use the same semaphore */
	resume_tasks((u32)sem);
}


/* mutex is a binary semaphore */
#define MUTEX_INIT	{ SEMAPHORE_INIT(1) }

static inline void
mutex_init(mutex_t *mut)
{
	semaphore_init(&mut->sem, 1);
}

/* returns 1 if managed to lock mutex
 * returns 0 if not */
static inline int
mutex_trylock(mutex_t *mut)
{
	return semaphore_trywait(&mut->sem);
}

/* blocks until it manage to lock mutex */
static inline void
mutex_lock(mutex_t *mut)
{
	semaphore_wait(&mut->sem);
}

/* unlock mutex */
static inline void
mutex_unlock(mutex_t *mut)
{
	semaphore_done(&mut->sem);
}

#endif	/* __SEMAPHORE_H */
