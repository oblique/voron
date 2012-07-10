OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
	_usr_stack_top = 0x80000000;

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

	_svc_stack_top = . + 0x2000;
	_fiq_stack_top = . + 0x2000;
	_irq_stack_top = . + 0x2000;
}
