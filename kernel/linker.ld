OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
	_ram_start = 0x80000000;
	_ram_end = 0xbfffffff;

	.text 0x80000000 :
	{
		_kernel_bin_start = .;
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

	. = ALIGN(4096);

	. = . + 0x2000;
	_svc_stack_top = .;
	. = . + 0x2000;
	_fiq_stack_top = .;
	. = . + 0x2000;
	_irq_stack_top = .;

	_kernel_bin_end = .;
}
