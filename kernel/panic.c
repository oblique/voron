#include <kernel.h>

void
panic(const char *fmt, ...)
{
	va_list ap;

	/* TODO: Shut down interrupts */
	kprintf("~~~ kernel panic ~~~\n");
	va_start(ap, fmt);
	kvprintf(fmt, ap);
	va_end(ap);
	for (;;);
}
