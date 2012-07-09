.section .text
.globl _start
_start:
	cpsid if		@ disable interrupts
	ldr sp, =_stack_top	@ set stack

	/* set leds off */
	mov r0, #0
	bl set_leds

	/* print booting message */
	ldr r0, =boot_msg
	bl kputs

	/* initialize vector table */
	bl init_vector_table

	bl kmain
	b .

.section .rodata
boot_msg:
	.asciz "\n\nVoron booting...\n\n"
