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

	/* initialize memory management */
	bl mm_init

	/* initialize interrupts */
	cps #CPS_FIQ		@ change to FIQ mode
	ldr sp, =_fiq_stack_top @ set FIQ stack

	cps #CPS_IRQ		@ change to IRQ mode
	ldr sp, =_irq_stack_top @ set IRQ stack

	cps #CPS_ABT	      @ change to Abort mode
	ldr sp, =_abort_stack_top @ set stack

	cps #CPS_SVC		@ change back to SVC mode
	bl irq_init		@ initialize controler
	cpsie if		@ enable interrupts

	bl dmtimer_init

	/* run constructors */
	ldr r0, =_kernel_ctors_start
	ldr r1, =_kernel_ctors_end
1:
	teq r0, r1
	beq 3f
	ldr r2, [r0], #4
	stmfd sp!, { r0, r1 }
	ldr lr, =2f
	mov pc, r2
2:
	ldmfd sp!, { r0, r1 }
	b 1b
3:

	@@ example of calling a system call with 1 argument
	mov r0, #1
	@@ the syscall number is 0
	mov r7, #0
	swi #0

	@@ example of calling a system call with 4 arguments
	mov r0, #1
	mov r1, #2
	mov r2, #3
	mov r3, #4
	@@ the syscall number is 0
	mov r7, #1
	swi #0

	@@ example of calling a system call with 5 arguments
	mov r0, #1
	mov r1, #2
	mov r2, #3
	mov r3, #4
	mov r4, #5
	@@ the syscall number is 0
	mov r7, #2
	swi #0

	@@ example of calling a system call with 7 arguments
	mov r0, #1
	mov r1, #2
	mov r2, #3
	mov r3, #4
	mov r4, #5
	mov r5, #6
	mov r6, #7
	@@ the syscall number is 0
	mov r7, #3
	swi #0

	@@ out of boundary syscall number example
	mov r7, #1000
	swi #0

	ldr r0, =kmain
	mov r1, #0
	bl kthread_create
	b schedule

.section .rodata
boot_msg:
	.asciz "\n\nVoron booting...\n\n"
