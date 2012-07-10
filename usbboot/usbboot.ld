OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
	.text 0x82000000 :
	{
		usbboot/usbboot.o (.text)
	}

	.image : ALIGN(4)
	{
		_image_start = .;
		usbboot/usbboot.o (.data)
	}
	_image_end = .;

	. = ALIGN(4);
	_usbboot_stack_top = . + 0x1000;
}
