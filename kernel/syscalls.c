#include <kernel.h>

long
sys_test_1_args(u32 a1)
{
	kprintf("sys_test_1_args(%d)\n", a1);
	return 0;
}


long
sys_test_4_args(u32 a1, u32 a2, u32 a3, u32 a4)
{
	kprintf("sys_test_4_args(%d, %d, %d, %d)\n", a1, a2, a3, a4);
	return 0;
}

long
sys_test_5_args(u32 a1, u32 a2, u32 a3, u32 a4, u32 a5)
{
	kprintf("sys_test_5_args(%d, %d, %d, %d, %d)\n", a1, a2, a3, a4, a5);
	return 0;
}

long
sys_test_7_args(u32 a1, u32 a2, u32 a3, u32 a4, u32 a5, u32 a6, u32 a7)
{
	kprintf("sys_test_7_args(%d, %d, %d, %d, %d, %d, %d)\n", a1, a2, a3, a4, a5, a6, a7);
	return 0;
}
