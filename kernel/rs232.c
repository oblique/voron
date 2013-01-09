#include <kernel.h>
#include <irq.h>
#include <rs232.h>
#include <uart.h>

/* UART3 */
#define UART_IRQ_NUM	(HW_IRQ(74))
static struct uart *uart = (struct uart*)0x48020000;

int
rs232_puts(const char *s)
{
	int ret = 0;

	while (s[ret]) {
		rs232_putchar(s[ret]);
		ret++;
	}

	return ret;
}

int
rs232_putchar(int c)
{
	if (c == '\n') {
		if (readl(&uart->ssr) & TX_FIFO_FULL) {
			while ((readl(&uart->lsr) & TX_FIFO_E) == 0)
				;
		}
		writel('\r', &uart->thr);
	}
	/* if UART TX FIFO is full */
	if (readl(&uart->ssr) & TX_FIFO_FULL) {
		/* while UART TX FIFO is not empty */
		while ((readl(&uart->lsr) & TX_FIFO_E) == 0)
			;
	}
	/* write the character */
	writel(c, &uart->thr);
	return c;
}

int
rs232_getchar(void)
{
	while ((readl(&uart->lsr) & RX_FIFO_E) == 0)
		;
	return readl(&uart->rhr);
}

static void
rs232_irq_handler(__unused u32 irq_num, __unused struct regs *regs)
{
	while(!(readl(&uart->iir) & IT_PENDING)) {
		if (readl(&uart->iir) & IT_TYPE_RHR)
			rs232_putchar(rs232_getchar());
	}
}

__attribute__((constructor))
void
rs232_init(void)
{
	irq_register(UART_IRQ_NUM, rs232_irq_handler);
	writel(RHR_IT, &uart->ier);
}
