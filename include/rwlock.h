#ifndef __RWLOCK_H
#define __RWLOCK_H

#include <inttypes.h>
#include <semaphore.h>

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

#endif	/* __RWLOCK_H */
