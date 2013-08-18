.section .text
.globl syscall_table
.globl syscall_table_end
syscall_table:
	.word sys_test_1_args
	.word sys_test_4_args
	.word sys_test_5_args
	.word sys_test_7_args
syscall_table_end:
	.word . - syscall_table
