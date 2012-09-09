#ifndef __SPINLOCK_H
#define __SPINLOCK_H

#define SPINLOCK_INIT	((spinlock_t)0)

typedef unsigned int spinlock_t;

static inline void spinlock_lock(spinlock_t *sl) {
	asm volatile (
		"1:			\n\t"
		"ldrex v1, [%0]		\n\t"
		"teq v1, #0		\n\t"
		"strexeq v1, %1, [%0]	\n\t"
		"teq v1, #0		\n\t"
		"bne 1b			\n\t"
		:
		: "r" (sl), "r" (0x80000000)
		: "v1", "memory"
	);
}

static inline void spinlock_unlock(spinlock_t *sl) {
	asm volatile (
		"str %1, [%0]"
		:
		: "r" (sl), "r" (0)
		: "memory"
	);
}

/* returns 1 if locked and 0 if not */
static inline int spinlock_trylock(spinlock_t *sl) {
	unsigned int tmp;

	asm volatile (
		"ldrex %0, [%1]		\n\t"
		"teq %0, #0		\n\t"
		"strexeq %0, %2, [%1]	\n\t"
		: "=r" (tmp)
		: "r" (sl), "r" (0x80000000)
		: "memory"
	);

	if (tmp == 0)
		return 1;
	else
		return 0;
}

static inline void INIT_SPINLOCK(spinlock_t *sl) {
	*sl = 0;
}

#endif	/* __SPINLOCK_H */
