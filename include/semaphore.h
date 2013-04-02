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
#define MUTEX_INIT		{ SEMAPHORE_INIT(1) }
#define MUTEX_INIT_LOCKED	{ SEMAPHORE_INIT(0) }

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
	spinlock_lock(&mut->sem.lock);
	/* if it's already unlocked, do nothing */
	if (mut->sem.counter == 1) {
		spinlock_unlock(&mut->sem.lock);
		return;
	}
	spinlock_unlock(&mut->sem.lock);
	semaphore_done(&mut->sem);
}


/* readers-writer lock
 * implemented with 'Passing the Baton' technique
 *
 * rules:
 * 1) if we have an awakened writer or writers in the queue then
 *    a new reader will be added in readers' queue.
 * 2) if all awakened readers are done then we awake a writer,
 *    even if we have readers in the queue.
 * 3) if the awakened writer and all writers in the queue are done
 *    then we awake all the readers. */
typedef struct {
	mutex_t lock;
	mutex_t writer_wait;
	mutex_t reader_wait;
	u32 nr_writers;
	u32 nr_readers;
	u32 nr_writers_queue;
	u32 nr_readers_queue;
} rwlock_t;

#define RWLOCK_INIT {				\
	MUTEX_INIT,				\
	MUTEX_INIT_LOCKED,			\
	MUTEX_INIT_LOCKED,			\
	0,					\
	0,					\
	0,					\
	0					\
}

static inline void
rwlock_init(rwlock_t *rwl)
{
	mutex_init(&rwl->lock);
	mutex_init(&rwl->writer_wait);
	mutex_lock(&rwl->writer_wait);
	mutex_init(&rwl->reader_wait);
	mutex_lock(&rwl->reader_wait);
	rwl->nr_writers = 0;
	rwl->nr_readers = 0;
	rwl->nr_writers_queue = 0;
	rwl->nr_readers_queue = 0;
}

static inline void
rwlock_rdlock(rwlock_t *rwl)
{
	mutex_lock(&rwl->lock);
	/* if we have awakened writer or writers in the queue then wait */
	if (rwl->nr_writers > 0 || rwl->nr_writers_queue > 0) {
		rwl->nr_readers_queue++;
		mutex_unlock(&rwl->lock);
		/* wait */
		mutex_lock(&rwl->reader_wait);
	}

	rwl->nr_readers++;

	/* if there is at least one reader in the queue,
	 * awake it (i.e. awake the next reader) */
	if (rwl->nr_readers_queue > 0) {
		rwl->nr_readers_queue--;
		mutex_unlock(&rwl->reader_wait);
	} else {
		mutex_unlock(&rwl->lock);
	}
}

/* returns 1 if managed to lock
 * returns 0 if not */
static inline int
rwlock_tryrdlock(rwlock_t *rwl)
{
	mutex_lock(&rwl->lock);
	/* if we have awakened writer or writers in the queue then we can't lock */
	if (rwl->nr_writers > 0 || rwl->nr_writers_queue > 0) {
		mutex_unlock(&rwl->lock);
		return 0;
	}

	rwl->nr_readers++;
	mutex_unlock(&rwl->lock);
	return 1;
}

static inline void
rwlock_wrlock(rwlock_t *rwl)
{
	mutex_lock(&rwl->lock);
	/* if we have an awakened writer or awakened readers then wait */
	if (rwl->nr_writers > 0 || rwl->nr_readers > 0) {
		rwl->nr_writers_queue++;
		mutex_unlock(&rwl->lock);
		/* wait */
		mutex_lock(&rwl->writer_wait);
	}

	rwl->nr_writers++;
	mutex_unlock(&rwl->lock);
}

/* returns 1 if managed to lock
 * returns 0 if not */
static inline int
rwlock_trywrlock(rwlock_t *rwl)
{
	mutex_lock(&rwl->lock);
	/* if we have an awakened writer or awakened readers then we can't lock */
	if (rwl->nr_writers > 0 || rwl->nr_readers > 0) {
		mutex_unlock(&rwl->lock);
		return 0;
	}

	rwl->nr_writers++;
	mutex_unlock(&rwl->lock);
	return 1;
}

static inline void
rwlock_unlock(rwlock_t *rwl)
{
	mutex_lock(&rwl->lock);

	/* a reader is done */
	if (rwl->nr_readers > 0) {
		rwl->nr_readers--;
	/* a writer is done */
	} else if (rwl->nr_writers > 0) {
		rwl->nr_writers--;
	}

	/* if all awakened readers are done and we have writers in the queue
	 * then awake a writer */
	if (rwl->nr_readers == 0 && rwl->nr_writers_queue > 0) {
		rwl->nr_writers_queue--;
		mutex_unlock(&rwl->writer_wait);
	/* if the awakened writer and all writers from the queue are done
	 * then awake a reader */
	} else if (rwl->nr_writers == 0 && rwl->nr_writers_queue == 0 &&
		   rwl->nr_readers_queue > 0) {
		rwl->nr_readers_queue--;
		mutex_unlock(&rwl->reader_wait);
	} else {
		mutex_unlock(&rwl->lock);
	}
}

#endif	/* __SEMAPHORE_H */
