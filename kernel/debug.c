#include <kernel.h>
#include <debug.h>

static u32 *gpio_wk7 = (u32*)0x4A31E058;
static u32 *gpio_wk8 = (u32*)0x4A31E05C;

void
set_led_d1(int on)
{
	u32 cur = readl(gpio_wk7) & 0xffff;
	if (on)
		cur |= 0x001B0000;
	else
		cur |= 0x00030000;
	writel(cur, gpio_wk7);
}

void
set_led_d2(int on)
{
	u32 cur = readl(gpio_wk8) & 0xffff0000;
	if (on)
		cur |= 0x001B;
	else
		cur |= 0x0003;
	writel(cur, gpio_wk8);
}

void
set_leds(int on)
{
	set_led_d1(on);
	set_led_d2(on);
}

static void
_kstackdump(void *sp)
{
	u32 *stack = sp;
	int i;

	for (i = 10; i >= 0; i--) {
		kprintf("%p: %p\n", &stack[i], stack[i]);
	}
}

__attribute__((__naked__))
void
kstackdump(void)
{
	asm volatile (
		"mov r0, sp		\n\t"
		"str lr, [sp, #-4]!	\n\t"
		"ldr lr, =1f		\n\t"
		"mov pc, %0		\n\t"
		"1:			\n\t"
		"ldr pc, [sp], #4	\n\t"
		:
		: "r" (_kstackdump)
		: "r0"
	);
}
