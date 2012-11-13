#include <kernel.h>
#include <list.h>
#include <dmtimer.h>
#include <sched.h>
#include <mmu.h>
#include <irq.h>
#include <p_modes.h>

#define MAX_HASH_ENT	64

struct task_struct *curr_task = NULL;
static struct list_head task_list_head;
static struct list_head ht_sleep[MAX_HASH_ENT];
static uatomic_t ms_counter = UATOMIC_INIT(0);
static int __sched_enabled = 0;

static pid_t
get_new_pid(void)
{
	static uatomic_t currpid = UATOMIC_INIT(0);
	return (pid_t)uatomic_add_return(1, &currpid);
}

static inline u32
hash_chan(u32 channel)
{
	int i;
	u32 hash = 0;

	for (i = 0; i < 32; ++i)
		hash = (channel & (1 << i)) + (hash << 8) + (hash << 12) - hash;

	return hash;
}

static void
task_remove(void)
{
	current->state = TASK_TERMINATE;
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
	task->regs.lr = (u32)task_remove;
	/* thread will run in System mode */
	task->regs.cpsr = CPS_SYS;

	/* add it to task list of the scheduler */
	if (sched_is_enabled()) {
		sched_disable();
		list_add(&task->list, &task_list_head);
		sched_enable();
	} else
		list_add(&task->list, &task_list_head);

	return 0;
}

void
schedule(void)
{
	if (current)
		current->scheduled = 0;
	/* trigger SGI */
	irq_trigger_sgi(1);
	/* make sure that we rescheduled */
	while (1) {
		if (current && current->state != TASK_TERMINATE &&
		    current->scheduled)
			break;
		asm volatile("wfi" : : : "memory");
	}
}

static void
__idle(void)
{
	while (1)
		asm volatile("wfi" : : : "memory");
}

static inline void
__switch_to(struct regs *regs, struct task_struct *new_curr)
{
	if (!new_curr) {
		/* if we we don't have any process
		 * make irq_ex return to __idle */
		regs->pc = (u32)__idle;
		/* we must add 4 because irq_ex subtracts 4 */
		regs->pc += 4;
	} else
		*regs = new_curr->regs;
	current = new_curr;
	/* data synchronization barrier */
	dsb();
	/* clear exclusive address access */
	asm volatile("clrex" : : : "memory");
}


static void
sched(struct regs *regs)
{
	struct list_head *iter, *curr_list;
	struct task_struct *task, *new_curr;

	if (list_empty(&task_list_head))
		return;

	if (current) {
		current->scheduled = 1;
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

		if (task->state == TASK_SLEEPING &&
		    task->sleep_reason == SLEEPR_SLEEP &&
		    task->wakeup_ms <= uatomic_read(&ms_counter)) {
			new_curr = task;
			new_curr->state = TASK_RUNNING;
			break;
		} else if (task->state == TASK_RUNNABLE) {
			new_curr = task;
			new_curr->state = TASK_RUNNING;
			break;
		}
	}

	if (current) {
		if (current->state == TASK_SLEEPING &&
		    current->sleep_reason == SLEEPR_SUSPEND) {
			int i = hash_chan(current->sleep_chan) % MAX_HASH_ENT;
			list_del(&current->list);
			list_add(&current->list, &ht_sleep[i]);
		} else if (current->state == TASK_RUNNING) {
			if (!new_curr)
				new_curr = current;
			else
				current->state = TASK_RUNNABLE;
		} else if (current->state == TASK_TERMINATE) {
			list_del(&current->list);
			kfree(current->stack_alloc);
			kfree(current);
		}
	}

	__switch_to(regs, new_curr);
}


void
sched_enable()
{
	u32 flags = irq_get_flags();
	irq_disable();
	writel(1, &__sched_enabled);
	irq_set_flags(flags);
}

void
sched_disable()
{
	u32 flags = irq_get_flags();
	irq_disable();
	writel(0, &__sched_enabled);
	irq_set_flags(flags);
}

int
sched_is_enabled()
{
	return readl(&__sched_enabled);
}

void
suspend_task(u32 channel)
{
	sched_disable();
	current->sleep_chan = channel;
	current->sleep_reason = SLEEPR_SUSPEND;
	current->state = TASK_SLEEPING;
	sched_enable();
	schedule();
}

void
suspend_task_no_schedule(u32 channel)
{
	/* caller *must* call sched_disable()
	 * before this function */
	current->sleep_chan = channel;
	current->sleep_reason = SLEEPR_SUSPEND;
	current->state = TASK_SLEEPING;
	/* caller *must* call sched_enable()
	 * and then schedule() after this function */
}

void
resume_tasks(u32 channel)
{
	struct list_head *iter, *n;
	struct task_struct *task;
	int i, sched_e;

	i = hash_chan(channel) % MAX_HASH_ENT;
	list_for_each_safe(iter, n, &ht_sleep[i]) {
		task = list_entry(iter, struct task_struct, list);
		if (task->sleep_chan == channel) {
			task->state = TASK_RUNNABLE;
			sched_e = sched_is_enabled();
			if (sched_e)
				sched_disable();
			list_del(iter);
			list_add(iter, &task_list_head);
			if (sched_e)
				sched_enable();
		}
	}
}

void
sleep(u32 seconds)
{
	sched_disable();
	current->wakeup_ms = uatomic_read(&ms_counter) + seconds * 1000;
	current->sleep_reason = SLEEPR_SLEEP;
	current->state = TASK_SLEEPING;
	sched_enable();
	/* schedule */
	schedule();
}

void
msleep(u32 milliseconds)
{
	sched_disable();
	/* TODO: if ms is smaller than SCHED_INT_MS
	 * do a loop and don't schedule */
	if (milliseconds < SCHED_INT_MS)
		milliseconds = SCHED_INT_MS;
	current->wakeup_ms = uatomic_read(&ms_counter) + milliseconds;
	current->sleep_reason = SLEEPR_SLEEP;
	current->state = TASK_SLEEPING;
	sched_enable();
	schedule();
}

static void
sched_handler(__unused int timer_id, struct regs *regs)
{
	uatomic_add(SCHED_INT_MS, &ms_counter);
	if (sched_is_enabled())
		sched(regs);
}

static void
manual_sched_handler(__unused u32 irq_num, struct regs *regs)
{
	if (sched_is_enabled())
		sched(regs);
}

__attribute__((__constructor__))
void
sched_init(void)
{
	size_t i;

	INIT_LIST_HEAD(&task_list_head);
	for (i = 0; i < ARRAY_SIZE(ht_sleep); i++)
		INIT_LIST_HEAD(&ht_sleep[i]);

	irq_register(1, manual_sched_handler);
	dmtimer_register(1, sched_handler, SCHED_INT_MS);
	sched_enable();
}
