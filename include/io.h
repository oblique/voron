#ifndef __IO_H
#define __IO_H

#include <inttypes.h>

/* data memory barrier */
#define dmb()	asm volatile("dmb" : : : "memory")
/* data synchronization barrier */
#define dsb()	asm volatile("dsb" : : : "memory")
/* instruction synchronization barrier */
#define isb()	asm volatile("isb" : : : "memory")

/* memory barrier */
#define mb()	dsb()
/* read memory barrier */
#define rmb()	dsb()
/* write memory barrier */
#define wmb()	dsb()

#define readl_relaxed(a)	(*(volatile u32*)(a))
#define readw_relaxed(a)	(*(volatile u16*)(a))
#define readb_relaxed(a)	(*(volatile u8*)(a))

#define writel_relaxed(v, a)	(*(volatile u32*)(a) = (u32)(v))
#define writew_relaxed(v, a)	(*(volatile u16*)(a) = (u16)(v))
#define writeb_relaxed(v, a)	(*(volatile u8*)(a) = (u8)(v))

#define readl(a)	({ u32 __r = readl_relaxed(a); rmb(); __r; })
#define readw(a)	({ u16 __r = readw_relaxed(a); rmb(); __r; })
#define readb(a)	({ u8  __r = readb_relaxed(a); rmb(); __r; })

#define writel(v, a)	({ wmb(); writel_relaxed(v, a); })
#define writew(v, a)	({ wmb(); writew_relaxed(v, a); })
#define writeb(v, a)	({ wmb(); writeb_relaxed(v, a); })

#endif	/* __IO_H */
