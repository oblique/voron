.section .text
.global init_vector_table
init_vector_table:
	mrc p15, 0, r0, c1, c0, 0	@ read CP15 SCTRL register
	bic r0, r0, #(1 << 13)		@ set V flag to 0
	mcr p15, 0, r0, c1, c0, 0	@ write CP15 SCTRL register

	ldr r0, =vector_table
	mcr p15, 0, r0, c12, c0, 0	@ set vector base address

	bx lr

vector_table:
	ldr pc, =reset
	ldr pc, =undefined
	ldr pc, =svc
	ldr pc, =prefetch_abort
	ldr pc, =data_abort
	mov pc, lr
	ldr pc, =irq
	ldr pc, =fiq
.ltorg


reset:
	b .

undefined:
	b .

svc:
	b .

prefetch_abort:
	b .

data_abort:
	b .

irq:
	b .

fiq:
	b .
