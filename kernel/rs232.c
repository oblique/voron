#include <inttypes.h>
#include <rs232.h>
#include <uart.h>
#include <io.h>

static struct uart *uart = (struct uart*)0x48020000;

int rs232_puts(const char *s) {
	int ret = 0;

	while (s[ret]) {
		rs232_putchar(s[ret]);
		ret++;
	}

	return ret;
}

int rs232_putchar(int c) {
	if (c == '\n') {
		while ((readl(&uart->lsr) & TX_FIFO_E) == 0)
			;
		writel('\r', &uart->thr);
	}
	/* while UART TX FIFO is not empty */
	while ((readl(&uart->lsr) & TX_FIFO_E) == 0)
		;
	/* write the character */
	writel(c, &uart->thr);
	return c;
}
