#include <kernel.h>
#include <alloc.h>
#include <string.h>

void kmain(void) {
	kprintf("voron initial stage\n\n");

	while (1)
		asm volatile("wfi");
}
