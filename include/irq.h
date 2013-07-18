#ifndef __IRQ_H
#define __IRQ_H

#include <kernel.h>

typedef void (*irq_callback_func)(u32 irq_num, struct regs *regs);

int irq_register(u32 irq_num, irq_callback_func func);
int irq_trigger_sgi(u32 irq_num);

/* board specific */
#define HW_IRQ(x)	(x + 32)
#define NR_IRQ		160

static inline void
irq_enable(void)
{
	asm volatile("cpsie if" : : : "memory");
}

static inline void
irq_disable(void)
{
	asm volatile("cpsid if" : : : "memory");
}

#define CPSR_MASK_IRQ	(1<<7)
#define CPSR_MASK_FIQ	(1<<6)

static inline u32
irq_get_flags()
{
	u32 val;
	asm volatile("mrs %0, cpsr" : "=r" (val) : : "memory");
	val &= CPSR_MASK_IRQ | CPSR_MASK_FIQ;
	return val;
}

static inline void
irq_set_flags(u32 flags)
{
	flags &= CPSR_MASK_IRQ | CPSR_MASK_FIQ;
	asm volatile (
		"mrs v1, cpsr		\n\t"
		"bic v1, v1, %0		\n\t"
		"orr v1, v1, %1		\n\t"
		"msr cpsr_c, v1		\n\t"
		:
		: "r" (CPSR_MASK_IRQ | CPSR_MASK_FIQ), "r" (flags)
		: "v1", "memory"
	);
}

#endif	/* __IRQ_H */
