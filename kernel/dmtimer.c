#include <kernel.h>
#include <irq.h>
#include <dmtimer.h>

#define DMT_OVF_ENA_FLAG		(1<<1)
#define DMT_TCLR_ST			(1<<0)
#define DMT_TCLR_AR			(1<<1)

#define DMT_1MS_TIOCP_SOFTRESET	(1<<1)
#define DMT_1MS_TIOCP_SMARTIDLE	(2<<3)
#define DMT_1MS_TISTAT_RESETDONE	(1<<0)

/* gptimer1, gptimer2, gptimer10 */
struct dm_gpt_1ms {
	u32 tidr;
	u32 _pad1[3];
	u32 tiocp_1ms_cfg;
	u32 tistat;
	u32 tisr;
	u32 tier;
	u32 twer;
	u32 tclr;
	u32 tcrr;
	u32 tldr;
	u32 ttgr;
	u32 twps;
	u32 tmar;
	u32 tcar1;
	u32 tsicr;
	u32 tcar2;
	u32 tpir;
	u32 tnir;
	u32 tcvr;
	u32 tocr;
	u32 towr;
};


#define DMT_TIOCP_SOFTRESET	(1<<0)
#define DMT_TIOCP_SMARTIDLE	(2<<2)

/* gptimer3, gptimer4, gptimer5, gptimer6
 * gptimer7, gptimer8, gptimer9, gptimer11
 */
struct dm_gpt {
	u32 tidr;
	u32 _pad1[3];
	u32 tiocp_cfg;
	u32 _pad2[4];
	u32 irqstatus_raw;
	u32 irqstatus;
	u32 irqenable_set;
	u32 irqenable_clr;
	u32 irqwakeen;
	u32 tclr;
	u32 tcrr;
	u32 tldr;
	u32 ttgr;
	u32 twps;
	u32 tmar;
	u32 tcar1;
	u32 tsicr;
	u32 tcar2;
};


#define DMT_1MS	(1<<0)
#define CM2_CLKSEL	(1<<24)

struct dmtimer {
	u32 flags;
	dmtimer_callback_func callback_func;
	union {
		u32 *mem;
		struct dm_gpt_1ms *dmt_1ms;
		struct dm_gpt *dmt;
	};
	/* L4PER CM2 register */
	u32 *cm2r;
} dmtimers[] = {
	{ DMT_1MS, NULL, { .mem = (u32*)0x4a318000 }, NULL },		  /* GPTIMER1 */
	{ DMT_1MS, NULL, { .mem = (u32*)0x48032000 }, (u32*)0x4a009438 }, /* GPTIMER2 */
	{	0, NULL, { .mem = (u32*)0x48034000 }, (u32*)0x4a009440 }, /* GPTIMER3 */
	{ 	0, NULL, { .mem = (u32*)0x48036000 }, (u32*)0x4a009448 }, /* GPTIMER4 */

	/* GPTIMER 5 - 8 are not implemented yet */
	{ 	0, NULL, { .mem = (u32*)NULL }, NULL }, /* GPTIMER5 */
	{ 	0, NULL, { .mem = (u32*)NULL }, NULL }, /* GPTIMER6 */
	{ 	0, NULL, { .mem = (u32*)NULL }, NULL }, /* GPTIMER7 */
	{ 	0, NULL, { .mem = (u32*)NULL }, NULL }, /* GPTIMER8 */

	{ 	0, NULL, { .mem = (u32*)0x4803e000 }, (u32*)0x4a009450 }, /* GPTIMER9 */
	{ DMT_1MS, NULL, { .mem = (u32*)0x48086000 }, (u32*)0x4a009428 }, /* GPTIMER10 */
	{ 	0, NULL, { .mem = (u32*)0x48088000 }, (u32*)0x4a009430 }  /* GPTIMER11 */
};



static void
dmtimer_irq_callback(u32 irq_num, struct regs *regs)
{
	u32 val;
	int id;

	if (irq_num < HW_IRQ(37) || irq_num > HW_IRQ(47))
		return;

	id = irq_num - HW_IRQ(37);

	if (dmtimers[id].mem == NULL)
		return;

	/* read type of event */
	if (dmtimers[id].flags & DMT_1MS)
		val = readl(&dmtimers[id].dmt_1ms->tisr);
	else
		val = readl(&dmtimers[id].dmt->irqstatus);

	if (dmtimers[id].callback_func)
		dmtimers[id].callback_func(id + 1, regs);

	/* clear event by writing 1 to the bits */
	if (dmtimers[id].flags & DMT_1MS)
		writel(val, &dmtimers[id].dmt_1ms->tisr);
	else
		writel(val, &dmtimers[id].dmt->irqstatus);
}

static int
reset_timer(int id)
{
	u32 val;

	if (id <= 0 || id > 11)
		return -EINVAL;

	id--;

	/* not implemented yet */
	if (dmtimers[id].mem == NULL)
		return -ENOSYS;

	if (dmtimers[id].flags & DMT_1MS) {
		/* request reset */
		writel(DMT_1MS_TIOCP_SOFTRESET, &dmtimers[id].dmt_1ms->tiocp_1ms_cfg);
		/* wait until reset is done */
		while (!(readl(&dmtimers[id].dmt_1ms->tistat) & DMT_1MS_TISTAT_RESETDONE))
			;
	} else {
		/* request reset */
		writel(DMT_TIOCP_SOFTRESET, &dmtimers[id].dmt->tiocp_cfg);
		/* wait until reset is done */
		while (readl(&dmtimers[id].dmt->tiocp_cfg) & DMT_TIOCP_SOFTRESET)
			;
	}

	/* enable 32KHz clock */
	if (dmtimers[id].cm2r) {
		val = readl(dmtimers[id].cm2r);
		val |= CM2_CLKSEL;
		writel(val, dmtimers[id].cm2r);
	}

	return 0;
}

int
dmtimer_trigger(int id)
{
	if (id <= 0 || id > 11)
		return -EINVAL;

	id--;

	/* not implemented yet */
	if (dmtimers[id].mem == NULL)
		return -ENOSYS;

	/* manually overflow the timer */
	if (dmtimers[id].flags & DMT_1MS)
		writel(0xffffffff, &dmtimers[id].dmt_1ms->tcrr);
	else
		writel(0xffffffff, &dmtimers[id].dmt->tcrr);

	return 0;
}

int
dmtimer_register(int id, dmtimer_callback_func func, u32 ms)
{
	int ret;
	u32 val;

	if (id <= 0 || id > 11 || ms == 0 || func == NULL)
		return -EINVAL;

	/* not implemented yet */
	if (dmtimers[id - 1].mem == NULL)
		return -ENOSYS;

	ret = reset_timer(id);
	if (ret)
		return ret;

	id--;
	ret = irq_register(HW_IRQ(37) + id, dmtimer_irq_callback);
	if (ret)
		return ret;
	dmtimers[id].callback_func = func;

	if (dmtimers[id].flags & DMT_1MS) {
		/* enable start-idle */
		writel(DMT_1MS_TIOCP_SMARTIDLE, &dmtimers[id].dmt_1ms->tiocp_1ms_cfg);

		/* set microseconds */
		val = 0xffffffff;
		val -= ms * 32;
		writel(val, &dmtimers[id].dmt_1ms->tldr);

		/* writing to TTGR causes TCRR to be loaded from TLDR */
		writel(1, &dmtimers[id].dmt_1ms->ttgr);
		/* enable overflow interrupt */
		writel(DMT_OVF_ENA_FLAG, &dmtimers[id].dmt_1ms->tier);
		/* start timer and enable autoreload */
		writel(DMT_TCLR_ST | DMT_TCLR_AR, &dmtimers[id].dmt_1ms->tclr);
	} else {
		/* enable start-idle */
		writel(DMT_TIOCP_SMARTIDLE, &dmtimers[id].dmt->tiocp_cfg);

		/* set microseconds */
		val = 0xffffffff;
		val -= ms * 32;
		writel(val, &dmtimers[id].dmt->tldr);

		/* writing to TTGR causes TCRR to be loaded from TLDR */
		writel(1, &dmtimers[id].dmt->ttgr);
		/* enable overflow interrupt */
		writel(DMT_OVF_ENA_FLAG, &dmtimers[id].dmt->irqenable_set);
		/* start timer and enable autoreload */
		writel(DMT_TCLR_ST | DMT_TCLR_AR, &dmtimers[id].dmt->tclr);
	}

	return 0;
}

void
dmtimer_init(void)
{
	int i;
	for (i = 1; i <= 11; i++)
		reset_timer(i);
}
