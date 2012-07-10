#include <p_modes.h>

.section .text
.globl _start
_start:
	cpsid if		@ disable interrupts
	ldr sp, =_svc_stack_top	@ set SVC stack

	/* set leds off */
	mov r0, #0
	bl set_leds

	/* print booting message */
	ldr r0, =boot_msg
	bl kputs

	/* initialize vector table */
	bl init_vector_table

	cps #CPS_FIQ		@ change to FIQ mode
	ldr sp, =_fiq_stack_top @ set FIQ stack

	cps #CPS_IRQ		@ change to IRQ mode
	ldr sp, =_irq_stack_top @ set IRQ stack

	cps #CPS_SVC		@ change to SVC mode
	cpsie if		@ enable interrupts

	/* example of calling a system call with at least 6 arguments */
	mov r0, #1
	mov r1, #2
	mov r2, #3
	mov r3, #4
	mov r4, #5
	mov r5, #6
	swi #0

	/* example of calling a system call with more than 6 arguments */
	mov r0, #7
	stmfd sp!, { r0 }
	mov r0, #1
	mov r1, #2
	mov r2, #3
	mov r3, #4
	mov r4, #5
	mov r5, #6
	stmfd sp!, { r0 - r5 }
	mov r0, sp
	swi #1
	add sp, sp, #(7 * 4)

	bl kmain
	b .

.section .rodata
boot_msg:
	.asciz "\n\nVoron booting...\n\n"
