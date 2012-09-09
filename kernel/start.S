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

	/* init .bss section */
	ldr r0, =_kernel_bss_start
	ldr r1, =_kernel_bss_end
	mov r2, #0
	mov r3, #0
1:
	stmia r0!, { r2, r3 }
	cmp r0, r1
	blo 1b

	/* initialize vector table */
	bl vector_table_init

	/* disable caches for now */
	mrc p15, 0, r0, c1, c0, 0
	bic r0, r0, #(1 << 12)          @ set I flag to 0 (disable instruction cache)
	bic r0, r0, #(1 << 2)		@ set C flag to 0 (disable data cache)
	mcr p15, 0, r0, c1, c0, 0

	cps #CPS_FIQ		@ change to FIQ mode
	ldr sp, =_fiq_stack_top @ set FIQ stack

	cps #CPS_IRQ		@ change to IRQ mode
	ldr sp, =_irq_stack_top @ set IRQ stack

	cps #CPS_SVC		@ change to SVC mode
	cpsie if		@ enable interrupts

	bl mm_init

	/* run constructors */
	ldr r0, =_kernel_ctors_start
	ldr r1, =_kernel_ctors_end
1:
	teq r0, r1
	beq 2f
	ldr r2, [r0], #4
	stmfd sp!, { r0, r1 }
	ldr lr, =.Lctor_ret
	mov pc, r2
.Lctor_ret:
	ldmfd sp!, { r0, r1 }
	b 1b
2:

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
	str r0, [sp, #-4]!
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
