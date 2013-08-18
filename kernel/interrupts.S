#include <p_modes.h>
#include <errno.h>

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
       sub sp, sp, #4	      @ make space for cpsr variable (cpsr of the previous mode)
	stmfd sp!, { lr }     @ save lr (pc (r15) of the previous mode)
	sub sp, sp, #8	      @ make space for sp (r13) and lr (r14) registers
	stmfd sp!, { r0 - r12 } @ save r0 until r12
	mrs r0, spsr		@ read spsr (cpsr of the previous mode)
	str r0, [sp, #(4 * 16)] @ save cpsr variable
	bic r0, r0, #0x60     @ clear F, T flags
	orr r0, r0, #(1 << 7) @ set I flag
	mrs r2, cpsr	      @ backup current cpsr
	msr cpsr_c, r0	      @ switch to previous mode
	mov r0, sp
	mov r1, lr
	msr cpsr, r2	      @ swith back to the current mode
	str r0, [sp, #(4 * 13)] @ save sp (r13)
	str r1, [sp, #(4 * 14)] @ save lr (r14)
	@@ restore destroyed registers
	ldr r0, [sp]
	ldr r1, [sp, #4]
	ldr r2, [sp, #8]
.endm

@@ restore registers
.macro RESTORE_ALL_EX
	ldr r0, [sp, #(4 * 13)] @ load sp (r13)
	ldr r1, [sp, #(4 * 14)] @ load lr (r14)
	ldr r2, [sp, #(4 * 16)] @ load cpsr variable
	msr spsr, r2	      @ restore spsr
	bic r2, r2, #0x60     @ clear F, T flags
	orr r2, r2, #(1 << 7) @ set I flag
	mrs r3, cpsr	      @ backup current cpsr
	msr cpsr_c, r2	      @ switch to previous mode
	mov sp, r0
	mov lr, r1
	msr cpsr, r3	      @ swith back to the current mode
	ldmfd sp!, { r0 - r12 }
	add sp, sp, #8	      @ release the space we had for sp (r13) and lr (r14)
	ldr lr, [sp], #4      @ load lr (pc (r15) of the previous mode)
	add sp, sp, #4	      @ release the space we had for cpsr variable
.endm

reset:
	b .


undefined_insn:
	movs pc, lr

@@ SWI/SVC handler use Linux ARM EABI calling convention:
@@ r0 - r6 registers are used for the syscall arguments, it can take up to 7 arguments.
@@ r7 register is used for the syscall number.
swi_ex:
	@@ save r8 - r12, lr registers
	stmfd sp!, { r8 - r12, lr }
	@@ save spsr register
	mrs r8, spsr
	str r8, [sp, #-4]!

	@@ calculate the number of system calls
	ldr r8, =syscall_table_end
	ldr r8, [r8]
	lsr r8, r8, #2

	@@ if syscall number is out of syscall table boundaries then return -ENOSYS
	cmp r7, r8
	movcs r0, #-ENOSYS
	bcs 2f

	@@ prepare arguments and call the system call
	stmfd sp!, { r4 - r6 }
	ldr lr, =1f
	ldr r8, =syscall_table
	ldr pc, [r8, r7, lsl #2]

	@@ return from SWI/SVC exception handler
1:
	add sp, sp, #(4 * 3)
2:
	ldr r8, [sp], #4
	msr spsr, r8
	ldmfd sp!, { r8 - r12, pc }^

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
