#include <inttypes.h>
#include <debug.h>
#include <io.h>

static u32 *gpio_wk7 = (u32*)0x4A31E058;
static u32 *gpio_wk8 = (u32*)0x4A31E05C;

void set_led_d1(int on) {
	u32 cur = readl(gpio_wk7) & 0xffff;
	if (on)
		cur |= 0x001B0000;
	else
		cur |= 0x00030000;
	writel(cur, gpio_wk7);
}

void set_led_d2(int on) {
	u32 cur = readl(gpio_wk8) & 0xffff0000;
	if (on)
		cur |= 0x001B;
	else
		cur |= 0x0003;
	writel(cur, gpio_wk8);
}

void set_leds(int on) {
	set_led_d1(on);
	set_led_d2(on);
}
