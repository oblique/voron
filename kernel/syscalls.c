#include <print.h>
#include <inttypes.h>

long sys_test_6_args(u32 a1, u32 a2, u32 a3, u32 a4, u32 a5, u32 a6) {
	kprintf("sys_test_6_args(%d, %d, %d, %d, %d, %d)\n", a1, a2, a3, a4, a5, a6);
	return 0;
}


struct __test_7_args {
	u32 a1;
	u32 a2;
	u32 a3;
	u32 a4;
	u32 a5;
	u32 a6;
	u32 a7;
};

long sys_test_7_args(struct __test_7_args *a) {
	kprintf("sys_test_7_args(%d, %d, %d, %d, %d, %d, %d)\n",
		a->a1, a->a2, a->a3, a->a4, a->a5, a->a6, a->a7);
	return 0;
}
