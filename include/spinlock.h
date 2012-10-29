#ifndef __SPINLOCK_H
#define __SPINLOCK_H

#include <io.h>

typedef struct {
	u32 lock;
} spinlock_t;

#define SPINLOCK_INIT	{ 0 }

static inline void
spinlock_lock(spinlock_t *sl)
{
	asm volatile (
		"1:			\n\t"
		"ldrex v1, [%0]		\n\t"
		"teq v1, #0		\n\t"
		/* wait for event if it's lock */
		"wfene			\n\t"
		"bne 1b			\n\t"
		"strex v1, %1, [%0]	\n\t"
		"teq v1, #0		\n\t"
		"bne 1b			\n\t"
		:
		: "r" (sl), "r" (0x80000000)
		: "v1", "memory"
	);
	dmb();
}

static inline void
spinlock_unlock(spinlock_t *sl)
{
	dmb();
	asm volatile (
		"str %1, [%0]"
		:
		: "r" (sl), "r" (0)
		: "memory"
	);
	dsb();
	/* signal event */
	asm volatile("sev" : : : "memory");
}

/* returns 1 if locked and 0 if not */
static inline int
spinlock_trylock(spinlock_t *sl)
{
	unsigned int tmp;

	asm volatile (
		"ldrex %0, [%1]		\n\t"
		"teq %0, #0		\n\t"
		"strexeq %0, %2, [%1]	\n\t"
		: "=&r" (tmp)
		: "r" (sl), "r" (0x80000000)
		: "memory"
	);

	if (tmp == 0) {
		dmb();
		return 1;
	} else
		return 0;
}

static inline void
INIT_SPINLOCK(spinlock_t *sl)
{
	sl->lock = 0;
}

#endif	/* __SPINLOCK_H */
