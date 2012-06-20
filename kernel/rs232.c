#include <inttypes.h>
#include <rs232.h>
#include <uart.h>

static volatile struct uart *uart = (struct uart*)0x48020000;

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
		while ((uart->lsr & TX_FIFO_E) == 0)
			;
		uart->thr = (u32)'\r';
	}
	/* while UART TX FIFO is not empty */
	while ((uart->lsr & TX_FIFO_E) == 0)
		;
	/* write the character */
	uart->thr = (u32)c;
	return c;
}
