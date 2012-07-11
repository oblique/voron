.section .text
.globl _start
_start:
	ldr sp, =_usbboot_stack_top
	stmfd sp!, { r0 - r12 }

	/* copy kernel image at _image_load_addr */
	ldr r0, =_image_start
	ldr r1, =_image_end
	ldr r2, =_image_load_addr

.L1:
	ldmia r0!, { r3 - r12 }
	stmia r2!, { r3 - r12 }
	cmp r0, r1
	blo .L1

	ldmfd sp!, { r0 - r12 }
	ldr pc, =_image_load_addr	@ jump to _image_load_addr
