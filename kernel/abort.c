#include <kernel.h>

void
abort_handler(struct regs *regs, int data)
{
	int i;

	if (data)
		kprintf("\n-DATA ABORT-\n");
	else
		kprintf("\n-PREFETCH ABORT-\n");

	for (i = 0; i <= 12; i++)
		kprintf("r%d: %p\n", i, regs->r[i]);
	kprintf("sp: %p\n", regs->sp);
	kprintf("lr: %p\n", regs->lr);
	kprintf("pc: %p\n", regs->pc - 8);
	kprintf("cpsr: %p\n", regs->cpsr);
}
