#ifndef __SCHED_H
#define __SCHED_H

#include <kernel.h>
#include <spinlock.h>
#include <list.h>

/* scheduler interval in milliseconds */
#define SCHED_INT_MS	10

typedef u32 pid_t;

typedef enum {
	TASK_TERMINATE,
	TASK_RUNNABLE,
	TASK_RUNNING,
	TASK_SLEEP
} task_state_t;


struct task_struct {
	pid_t pid;
	task_state_t state;
	struct regs regs;
	struct list_head list;
	spinlock_t *lock;
	void *stack_alloc;
	u32 wakeup_ms;
};


extern struct task_struct *curr_task;

static inline struct task_struct **
current_task(void)
{
	return &curr_task;
}

#define current (*current_task())

void sleep(u32 seconds);
void msleep(u32 milliseconds);
int kthread_create(void (*routine)(void *), void *arg);
void schedule(void);

#endif	/* __SCHED_H */
