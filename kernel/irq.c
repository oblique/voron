#include <kernel.h>
#include <gic.h>
#include <irq.h>

int
irq_register(u32 irq_num, irq_callback_func func)
{
	return gic_register(irq_num, func);
}

int
irq_trigger_sgi(u32 irq_num)
{
	return gic_trigger_sgi(irq_num);
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
