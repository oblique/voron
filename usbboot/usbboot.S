.section .text
.globl _start
_start:
	ldr sp, =_usbboot_stack_top
	push { r0 - r12 }

	/* copy kernel image at 0x80008000 */
	ldr r0, =_image_start
	ldr r1, =_image_end
	ldr r2, =0x80008000

.L1:
	ldmia r0!, { r3 - r12 }
	stmia r2!, { r3 - r12 }
	cmp r0, r1
	blo .L1

	pop { r0 - r12 }
	ldr pc, =0x80008000	@ jump to 0x80008000
