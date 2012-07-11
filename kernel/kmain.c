#include <print.h>
#include <debug.h>

void kmain(void) {
	kprintf("voron initial stage\n");

	while (1)
		asm volatile("wfi");
}
