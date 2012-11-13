#ifndef __SEMAPHORE_H
#define __SEMAPHORE_H

#include <irq.h>
#include <io.h>
#include <spinlock.h>
#include <sched.h>

typedef struct {
	u32 counter;
	spinlock_t lock;
} semaphore_t;

typedef struct {
	semaphore_t sem;
} mutex_t;

#define SEMAPHORE_INIT(v)	{ (v), SPINLOCK_INIT }

static inline void
semaphore_init(semaphore_t *sem, u32 value)
{
	sem->counter = value;
	INIT_SPINLOCK(&sem->lock);
}

/* returns 1 if managed to decreased the counter
 * returns 0 if not */
static inline int
semaphore_trywait(semaphore_t *sem)
{
	spinlock_lock(&sem->lock);
	if (sem->counter == 0) {
		spinlock_unlock(&sem->lock);
		return 0;
	}
	sem->counter--;
	spinlock_unlock(&sem->lock);
	return 1;
}

/* blocks until it manage to decrease the counter */
static inline void
semaphore_wait(semaphore_t *sem)
{
	while (1) {
		spinlock_lock(&sem->lock);
		if (sem->counter == 0) {
			sched_disable();
			suspend_task_no_schedule((u32)sem);
			spinlock_unlock(&sem->lock);
			sched_enable();
			schedule();
			continue;
		}
		sem->counter--;
		spinlock_unlock(&sem->lock);
		break;
	}
}

/* increases the counter and resumes other tasks */
static inline void
semaphore_done(semaphore_t *sem)
{
	spinlock_lock(&sem->lock);
	sem->counter++;
	resume_tasks((u32)sem);
	spinlock_unlock(&sem->lock);
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
