#ifndef __ATOMIC_H
#define __ATOMIC_H

#include <inttypes.h>

typedef struct {
	u32 counter;
} uatomic_t;

#define UATOMIC_INIT(i)		{ (i) }
#define uatomic_read(v)		(__uatomic_read(v).counter)
#define uatomic_add_return(i, v)	(__uatomic_add_return(i, v).counter)

static inline uatomic_t
__uatomic_read(uatomic_t *v)
{
	uatomic_t r;
	asm volatile (
		"1:			\n\t"
		"ldrex %0, [%1]		\n\t"
		"strex v1, %0, [%1]	\n\t"
		"teq v1, #0		\n\t"
		"bne 1b			\n\t"
		/* '&' guarantees that 'r' variable will have its own
		 * register so GCC will not produce assembly code
		 * like this one: ldrex r5, [r5] */
		: "=&r" (r)
		: "r" (v)
		: "v1", "memory"
	);
	return r;
}

static inline void
uatomic_set(uatomic_t *v, u32 i)
{
	asm volatile (
		"1:			\n\t"
		"ldrex v1, [%1]		\n\t"
		"strex v1, %0, [%1]	\n\t"
		"teq v1, #0		\n\t"
		"bne 1b			\n\t"
		:
		: "r" (i), "r" (v)
		: "v1", "memory"
	);
}

static inline void
uatomic_add(u32 i, uatomic_t *v)
{
	asm volatile (
		"1:			\n\t"
		"ldrex v1, [%1]		\n\t"
		"add v1, v1, %0		\n\t"
		"strex v2, v1, [%1]	\n\t"
		"teq v2, #0		\n\t"
		"bne 1b			\n\t"
		:
		: "r" (i), "r" (v)
		: "v1", "v2", "memory"
	);
}

static inline uatomic_t
__uatomic_add_return(u32 i, uatomic_t *v)
{
	uatomic_t r;
	asm volatile (
		"1:			\n\t"
		"ldrex %0, [%2]		\n\t"
		"add %0, %0, %1		\n\t"
		"strex v1, %0, [%2]	\n\t"
		"teq v1, #0		\n\t"
		"bne 1b			\n\t"
		: "=&r" (r)
		: "r" (i), "r" (v)
		: "v1", "memory"
	);
	return r;
}

#endif	/* __ATOMIC_H */
