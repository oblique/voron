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
static uatomic_t ms_counter = UATOMIC_INIT(0);

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
	u32 ms = uatomic_read(&ms_counter);
	/* this guarantees that will loop until scheduler will executed */
	while (ms == uatomic_read(&ms_counter)) {
		/* trigger scheduler timer */
		dmtimer_trigger(1);
		/* wait for interrupt */
		asm volatile("wfi");
	}
}

static void
__idle(void)
{
	while (1)
		asm volatile("wfi");
}

static void
__switch_to(struct regs *regs, struct task_struct *new_curr)
{
	if (!new_curr) {
		/* if we we don't have any process
		 * make irq_ex return to __idle */
		regs->pc = (u32)&__idle;
		/* we must add 4 because irq_ex subtracts 4 */
		regs->pc += 4;
	} else
		*regs = new_curr->regs;
	current = new_curr;
	dsb();
	asm volatile("clrex");
}


static void
sched(struct regs *regs)
{
	struct list_head *iter, *curr_list;
	struct task_struct *task, *new_curr;

	/* TODO: if scheduler is triggered by schedule()
	 * then add the correct value */
	uatomic_add(SCHED_INT_MS, &ms_counter);

	if (list_empty(&task_list_head))
		return;

	if (current) {
		if (current->state != TASK_TERMINATE)
			current->regs = *regs;
		curr_list = &current->list;
	} else
		curr_list = &task_list_head;

	new_curr = NULL;

	list_for_each(iter, curr_list) {
		if (iter == &task_list_head)
			continue;

		task = list_entry(iter, struct task_struct, list);

		if (task->state == TASK_SLEEP && task->wakeup_ms <= uatomic_read(&ms_counter)) {
			new_curr = task;
			new_curr->state = TASK_RUNNING;
			break;
		} else if (task->state == TASK_RUNNABLE) {
			new_curr = task;
			new_curr->state = TASK_RUNNING;
			break;
		}
	}

	if (!new_curr && current && current->state == TASK_RUNNING)
		new_curr = current;

	if (current && current->state == TASK_TERMINATE) {
		spinlock_lock(current->lock);
		list_del(&current->list);
		spinlock_unlock(current->lock);
		kfree(current->stack_alloc);
		kfree(current);
	} else if (current && current != new_curr && current->state == TASK_RUNNING)
		current->state = TASK_RUNNABLE;

	__switch_to(regs, new_curr);
}

void
sleep(u32 seconds)
{
	current->state = TASK_SLEEP;
	current->wakeup_ms = uatomic_read(&ms_counter) + seconds * 1000;
	schedule();
}

void
msleep(u32 milliseconds)
{
	current->state = TASK_SLEEP;
	/* TODO: if ms is smaller than SCHED_INT_MS
	 * do a loop and don't schedule */
	if (milliseconds < SCHED_INT_MS)
		milliseconds = SCHED_INT_MS;
	current->wakeup_ms = uatomic_read(&ms_counter) + milliseconds;
	schedule();
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
	dmtimer_register(1, sched_handler, SCHED_INT_MS);
}
