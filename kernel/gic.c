#include <kernel.h>
#include <irq.h>
#include <gic.h>

#define IAR_ID(x)	(x & 0x3ff)

/* GICv1 CPU interface registers */
struct gicc {
	u32 ctlr;
	u32 pmr;
	u32 bpr;
	u32 iar;
	u32 eoir;
	u32 rpr;
	u32 hppir;
	u32 abpr;
	u32 _pad[55];
	u32 iidr;
};

#define MAX_IT_LINES_NUMBER 32

/* GICv1 Distributor registers */
struct gicd {
	u32 ctlr;
	u32 typer;
	u32 iidr;
	u32 _pad1[29];
	u32 igrour[MAX_IT_LINES_NUMBER];
	u32 isenabler[MAX_IT_LINES_NUMBER];
	u32 icenabler[MAX_IT_LINES_NUMBER];
	u32 ispendr[MAX_IT_LINES_NUMBER];
	u32 icpendr[MAX_IT_LINES_NUMBER];
	u32 isactiver[MAX_IT_LINES_NUMBER];
	u32 _pad2[32];
	u32 ipriorityr[8 * MAX_IT_LINES_NUMBER];
	u32 itargetsr[8 * MAX_IT_LINES_NUMBER];
	u32 icfgr[2 * MAX_IT_LINES_NUMBER];
	u32 _pad3[128];
	u32 sgir;
};

static struct gicc *gicc = (struct gicc*)0x48240100;
static struct gicd *gicd = (struct gicd*)0x48241000;
static irq_callback_func irq_handlers[NUM_OF_IRQ];


int
gic_register(u32 irq_num, irq_callback_func func)
{
	int i;
	u32 val;

	if (irq_num >= NUM_OF_IRQ)
		return -EINVAL;

	/* set callback function */
	irq_handlers[irq_num] = func;

	/* enable irq number */
	i = irq_num / 32;
	val = readl(&gicd->isenabler[i]);
	val |= 1 << (irq_num % 32);
	writel(val, &gicd->isenabler[i]);

	/* send the interrupt to CPU0 */
	i = irq_num / 4;
	val = readl(&gicd->itargetsr[i]);
	val &= ~(0xff << (8 * (irq_num % 4)));
	val |= 1 << (8 * (irq_num % 4));
	writel(val, &gicd->itargetsr[i]);

	return 0;
}

void
gic_handler(struct regs *regs)
{
	u32 iar, id;

	/* read interrupt id */
	iar = readl(&gicc->iar);
	id = IAR_ID(iar);

	/* call handler */
	if (id < NUM_OF_IRQ && irq_handlers[id] != NULL)
		irq_handlers[id](id, regs);

	/* end of interrupt */
	writel(iar, &gicc->eoir);
}

void
gic_init(void)
{
	/* disable gic cpu interface */
	writel(0, &gicc->ctlr);
	/* disable gic distributor */
	writel(0, &gicd->ctlr);

	/* make sure that software interrupts are enabled */
	writel(0xffff, &gicd->isenabler[0]);

	/* set lower priority level */
	writel(0xf0, &gicc->pmr);
	/* enable gic cpu interface */
	writel(1, &gicc->ctlr);
	/* enable gic cpu distributor */
	writel(1, &gicd->ctlr);
}
