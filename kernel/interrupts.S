.section .text
.global init_vector_table
init_vector_table:
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
	ldr lr, =.Lsyscall_ret
	ldr r7, =syscall_table
	ldr pc, [r7, r6, lsl #2]

.Lsyscall_ret:
	add sp, sp, #8
	ldr r6, [sp], #4
	msr spsr, r6
	ldmfd sp!, { r6 - r12, pc }^


prefetch_abort:
	subs pc, lr, #4


data_abort:
	subs pc, lr, #8


irq_ex:
	sub lr, lr, #4
	str lr, [sp, #-4]!
	mov r0, #1
	bl set_led_d1
	ldmfd sp!, { pc }^


fiq_ex:
	sub lr, lr, #4
	str lr, [sp, #-4]!
	mov r0, #1
	bl set_led_d2
	ldmfd sp!, { pc }^
