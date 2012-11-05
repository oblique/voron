#include <kernel.h>
#include <sched.h>
#include <semaphore.h>

semaphore_t sem = SEMAPHORE_INIT(2);

static void
thread_func(void *arg)
{
	u32 i = 0, n = (u32)arg;
	while (1) {
		if (i == 5 * n) {
			kprintf("thread %d waiting semaphore\n", n);
			semaphore_wait(&sem);
			kprintf("thread %d locked semaphore\n", n);
		}
		kprintf("thread %d  i: %d\n", n, i);
		msleep(500);
		i++;
		if (i == 20 * n) {
			i = 0;
			semaphore_done(&sem);
			kprintf("thread %d released semaphore\n", n);
		}
	}
}

void
kmain(void)
{
	kprintf("voron initial stage\n\n");
	kthread_create(thread_func, (void*)1);
	kthread_create(thread_func, (void*)2);
	kthread_create(thread_func, (void*)3);
}
