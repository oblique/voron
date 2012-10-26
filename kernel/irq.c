#include <kernel.h>
#include <gic.h>
#include <irq.h>

int
irq_register(u32 irq_num, irq_callback_func func)
{
	return gic_register(irq_num, func);
}

void
irq_handler(struct regs *regs)
{
	gic_handler(regs);
}

void
irq_init(void)
{
	gic_init();
}
