#ifndef __GIC_H
#define __GIC_H

#include <kernel.h>
#include <irq.h>

int gic_register(u32 irq_num, irq_callback_func func);
void gic_handler(struct regs *regs);
int gic_trigger_sgi(u32 irq_num);
void gic_init(void);

#endif	/* __GIC_H */
