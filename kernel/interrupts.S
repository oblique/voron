#include <p_modes.h>

.section .text
.global vector_table_init
vector_table_init:
	mrc p15, 0, r0, c1, c0, 0	@ read CP15 SCTRL register
	bic r0, r0, #(1 << 13)		@ set V flag to 0 (disable high vectors)
	mcr p15, 0, r0, c1, c0, 0	@ write CP15 SCTRL register

	ldr r0, =vector_table
	mcr p15, 0, r0, c12, c0, 0	@ set vector base address (VBAR)

	bx lr


.balign 32	@ the 5 least-significant bits of VBAR are reserved (i.e. 32 bytes alignment)
vector_table:
	ldr pc, =reset
	ldr pc, =undefined_insn
	ldr pc, =swi_ex
	ldr pc, =prefetch_abort
	ldr pc, =data_abort
	b .			@ not assigned
	ldr pc, =irq_ex
	ldr pc, =fiq_ex
.ltorg


@@ save all the registers
.macro SAVE_ALL_EX
	stmfd sp!, { lr }     @ save lr (pc (r15) of the previous mode)
	sub sp, sp, #8	      @ make space for sp (r13) and lr (r14) registers
	stmfd sp!, { r0 - r12 } @ save r0 until r12
	mrs r0, spsr	      @ save spsr (cpsr of the previus mode)
	str r0, [sp, #-4]!    @ save cpsr
	bic r0, r0, #0x60     @ clear F, T flags
	orr r0, r0, #(1 << 7) @ set I flag
	mrs r2, cpsr	      @ backup current cpsr
	msr cpsr_c, r0	      @ switch to previous mode
	mov r0, sp
	mov r1, lr
	msr cpsr, r2	      @ swith back to the current mode
	str r0, [sp, #(4 * 14)] @ save sp (r13)
	str r1, [sp, #(4 * 15)] @ save lr (r14)

	ldr r0, [sp, #4]      @ restore destroyed registers
	ldr r1, [sp, #8]
	ldr r2, [sp, #12]
.endm

@@ restore registers
.macro RESTORE_ALL_EX
	ldr r1, [sp, #(4 * 14)] @ load sp (r13)
	ldr r2, [sp, #(4 * 15)] @ load lr (r14)
	ldr r0, [sp]
	bic r0, r0, #0x60
	orr r0, r0, #(1 << 7) @ get the previous mode number
	mrs r3, cpsr
	msr cpsr_c, r0	      @ switch to previous mode
	mov sp, r1
	mov lr, r2
	msr cpsr, r3	      @ swith back to the current mode
	ldr r0, [sp], #4      @ load spsr (cpsr of the previus mode)
	msr spsr, r0
	ldmfd sp!, { r0 - r12 }
	add sp, sp, #8	      @ release the space we had for sp (r13) and lr (r14)
	ldr lr, [sp], #4      @ load lr (pc (r15) of the previous mode)
.endm


reset:
	b .


undefined_insn:
	movs pc, lr


/* system call number is the swi number
 * if system call takes less than 7 arguments
 * we use r0 until r5 from userland to pass them
 * if it takes more than 6 arguments we pass a
 * pointer of the arguments at r0
 */
swi_ex:
	stmfd sp!, { r6 - r12, lr }
	mrs r6, spsr
	str r6, [sp, #-4]!

	ldr r6, [lr, #-4]
	bic r6, r6, #0xff000000

	stmfd sp!, { r4, r5 }
	ldr lr, =1f
	ldr r7, =syscall_table
	ldr pc, [r7, r6, lsl #2]
1:
	add sp, sp, #8
	ldr r6, [sp], #4
	msr spsr, r6
	ldmfd sp!, { r6 - r12, pc }^


prefetch_abort:
	@@ save registers
	SAVE_ALL_EX
	@@ set led d1 on
	mov r0, #1
	bl set_led_d1
	@@ call handler
	mov r0, sp		@ pass a pointer to the registers
	mov r1, #0		@ 0 = prefetch
	bl abort_handler	@ call handler

	b .		      @ freeze
	@subs pc, lr, #4


data_abort:
	@@ save registers
	SAVE_ALL_EX
	@@ set led d2 on
	mov r0, #1
	bl set_led_d2
	@@ call handler
	mov r0, sp		@ pass a pointer to the registers
	mov r1, #1		@ 1 = data
	bl abort_handler	@ call handler

	b .		      @ freeze
	@subs pc, lr, #8


irq_ex:
	@@ save registers
	SAVE_ALL_EX
	@@ call handler
	mov r0, sp	      @ pass a pointer to the registers
	bl irq_handler	      @ call irq_handler
	@@ restore registers
	RESTORE_ALL_EX
	@@ exit exception
	subs pc, lr, #4


fiq_ex:
	sub lr, lr, #4
	str lr, [sp, #-4]!
	mov r0, #1
	bl set_led_d2
	ldmfd sp!, { pc }^
