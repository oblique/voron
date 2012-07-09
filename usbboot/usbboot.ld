OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
	.text 0x82000000 :
	{
		usbboot/usbboot.o (.text)

		_image_start = .;
		INCLUDE "usbboot/kernel_image.ld";
		_image_end = .;
	}

	. = ALIGN(4);
	_usbboot_stack_top = . + 0x1000;
}
