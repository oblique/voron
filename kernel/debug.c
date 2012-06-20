#include <inttypes.h>

static volatile u32 *gpio_wk7 = (u32*)0x4A31E058;
static volatile u32 *gpio_wk8 = (u32*)0x4A31E05C;

void set_led_d1(int on) {
	u32 cur = *gpio_wk7 & 0xffff;
	if (on)
		*gpio_wk7 = cur | 0x001B0000;
	else
		*gpio_wk7 = cur | 0x00030000;
}

void set_led_d2(int on) {
	u32 cur = *gpio_wk8 & 0xffff0000;
	if (on)
		*gpio_wk8 = cur | 0x001B;
	else
		*gpio_wk8 = cur | 0x0003;
}

void set_leds(int on) {
	set_led_d1(on);
	set_led_d2(on);
}
