OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
	. = 0x82000000;
	. = ALIGN(4);
	.text :
	{
		kernel/start.o (.text)
		*(.text)
	}

	.data : { *(.data) }

	.bss : { *(.bss) }

	. = . + 0x2000;
	_stack_top = .;
}
