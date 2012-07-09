OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
	.text 0x80008000 :
	{
		kernel/start.o (.text)
		*(.text)
	}

	.data : ALIGN(4096)
	{
		*(.data)
	}

	.rodata : ALIGN(4096)
	{
		*(.rodata)
	}

	.bss : ALIGN(4096)
	{
		*(.bss)
	}

	_stack_top = . + 0x2000;
}
