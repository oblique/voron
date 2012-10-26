#ifndef __IRQ_H
#define __IRQ_H

#include <kernel.h>

typedef void (*irq_callback_func)(u32 irq_num, struct regs *regs);

int irq_register(u32 irq_num, irq_callback_func func);

/* board specific */
#define HW_IRQ(x)	(x + 32)
#define NUM_OF_IRQ	160

#endif	/* __IRQ_H */
