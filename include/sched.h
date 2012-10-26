#ifndef __SCHED_H
#define __SCHED_H

#include <kernel.h>
#include <spinlock.h>
#include <list.h>

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
};


extern struct task_struct *curr_task;

static inline struct task_struct **
current_task(void)
{
	return &curr_task;
}

#define current (*current_task())

int kthread_create(void (*routine)(void *), void *arg);
void schedule(void);

#endif	/* __SCHED_H */
