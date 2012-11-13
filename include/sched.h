#ifndef __SCHED_H
#define __SCHED_H

#include <kernel.h>
#include <list.h>

/* scheduler interval in milliseconds */
#define SCHED_INT_MS	10

typedef u32 pid_t;

typedef enum {
	TASK_TERMINATE,
	TASK_RUNNABLE,
	TASK_RUNNING,
	TASK_SLEEPING
} task_state_t;

typedef enum {
	SLEEPR_SLEEP,
	SLEEPR_SUSPEND
} sleep_reason_t;


struct task_struct {
	pid_t pid;
	task_state_t state;
	sleep_reason_t sleep_reason;
	u32 sleep_chan;
	volatile int scheduled;
	struct regs regs;
	struct list_head list;
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
void sched_disable();
void sched_enable();
int sched_is_enabled();
void suspend_task(u32 channel);
void suspend_task_no_schedule(u32 channel);
void resume_tasks(u32 channel);

#endif	/* __SCHED_H */
