OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
	_image_load_addr = 0x80000000;

	.text 0x82000000 :
	{
		usbboot/usbboot.o (.text)
	}

	.image : ALIGN(4)
	{
		_image_start = .;
		usbboot/kernel_image.o (.data)
	}
	. = ALIGN(4);
	_image_end = .;

	_usbboot_stack_top = . + 0x1000;
}
