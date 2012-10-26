#include <kernel.h>
#include <list.h>
#include <spinlock.h>
#include <dmtimer.h>
#include <sched.h>
#include <mmu.h>
#include <p_modes.h>

struct task_struct *curr_task = NULL;
static struct list_head task_list_head;
static spinlock_t task_struct_lock = SPINLOCK_INIT;

static pid_t
get_new_pid(void)
{
	static pid_t currpid = 0;
	static spinlock_t lock = SPINLOCK_INIT;
	pid_t new_pid;

	spinlock_lock(&lock);
	new_pid = ++currpid;
	spinlock_unlock(&lock);

	return new_pid;
}

static void
task_remove(void)
{
	current->state = TASK_TERMINATE;
	/* force schedule */
	schedule();
}

int
kthread_create(void (*routine)(void *), void *arg)
{
	struct task_struct *task;

	task = kmalloc(sizeof(*task));
	if (!task)
		return -ENOMEM;

	/* allocate stack */
	task->stack_alloc = kmalloc(PAGE_SIZE);
	if (!task->stack_alloc) {
		kfree(task);
		return -ENOMEM;
	}

	task->state = TASK_RUNNABLE;
	task->pid = get_new_pid();
	task->lock = &task_struct_lock;
	memset(&task->regs, 0, sizeof(task->regs));
	/* set thread stack */
	task->regs.sp = (u32)task->stack_alloc;
	task->regs.sp += PAGE_SIZE;
	/* set argument */
	task->regs.r0 = (u32)arg;
	/* set the function that new thread will execute
	 * we must add 4 because irq_ex will subtract 4 */
	task->regs.pc = (u32)routine;
	task->regs.pc += 4;
	/* set return address */
	task->regs.lr = (u32)&task_remove;
	/* thread will run in System mode */
	task->regs.cpsr = CPS_SYS;

	/* add it to task list of the scheduler */
	spinlock_lock(task->lock);
	list_add(&task->list, &task_list_head);
	spinlock_unlock(task->lock);

	return 0;
}

void
schedule(void)
{
	/* trigger scheduler timer */
	dmtimer_trigger(1);
	while(1)
		asm volatile("wfi");
}

static void
sched(struct regs *regs)
{
	if (list_empty(&task_list_head))
		return;

	if (current) {
		struct list_head *iter;
		struct task_struct *task, *prev;

		if (current->state != TASK_TERMINATE)
			current->regs = *regs;

		prev = current;
		list_for_each(iter, &prev->list) {
			if (iter == &task_list_head)
				continue;
			task = list_entry(iter, struct task_struct, list);
			if (task->state == TASK_RUNNABLE) {
				current = task;
				break;
			}
		}

		if (iter == &prev->list && prev->state != TASK_RUNNING)
			current = NULL;

		if (prev->state == TASK_TERMINATE) {
			spinlock_lock(prev->lock);
			list_del(&prev->list);
			spinlock_unlock(prev->lock);
			kfree(prev->stack_alloc);
			kfree(prev);
		} else if (prev != current)
			prev->state = TASK_RUNNABLE;
	} else
		current = list_first_entry(&task_list_head, struct task_struct, list);

	if (current) {
		current->state = TASK_RUNNING;
		*regs = current->regs;
	}
}

static void
sched_handler(__unused int timer_id, struct regs *regs)
{
	sched(regs);
}

__attribute__((__constructor__))
void
sched_init(void)
{
	INIT_LIST_HEAD(&task_list_head);
	dmtimer_register(1, sched_handler, 10);
}
