.text
.globl _start
_start:
	ldr sp, =_stack_top
	bl kmain
	b .
