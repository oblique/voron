#include <kernel.h>
#include <irq.h>

void
panic(const char *fmt, ...)
{
	va_list ap;

	irq_disable();
	kprintf("~~~ kernel panic ~~~\n");
	va_start(ap, fmt);
	kvprintf(fmt, ap);
	va_end(ap);
	for (;;);
}
