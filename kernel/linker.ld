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

	.init_array : ALIGN(4096)
	{
		_kernel_ctors_start = .;
		*(.init_array)
		_kernel_ctors_end = .;
	}

	.rodata : ALIGN(4096)
	{
		*(.rodata)
	}

	.bss : ALIGN(4096)
	{
		_kernel_bss_start = .;
		*(.bss)
	}

	. = ALIGN(4096);
	_kernel_bss_end = .;

	_kernel_heap_start = .;
	. = . + 0x10000000;
	_kernel_heap_end = .;

	. = . + 0x2000;
	_svc_stack_top = .;
	. = . + 0x2000;
	_fiq_stack_top = .;
	. = . + 0x2000;
	_irq_stack_top = .;
	. = . + 0x2000;
	_abort_stack_top = .;


	_kernel_bin_end = .;
}
