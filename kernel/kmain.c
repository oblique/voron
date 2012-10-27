#include <kernel.h>
#include <alloc.h>
#include <string.h>
#include <gic.h>
#include <sched.h>

static void
thread_func(void *arg)
{
	u32 n = (u32)arg;
	while (1) {
		kprintf("thread %d\n", n);
		msleep(500);
	}
}

void
kmain(void)
{
	kprintf("voron initial stage\n\n");
	kthread_create(thread_func, (void*)1);
	kthread_create(thread_func, (void*)2);
}
