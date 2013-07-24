#include <kernel.h>
#include <rs232.h>

int
kputs(const char *s)
{
	return rs232_puts(s);
}

int
kputchar(int c)
{
	return rs232_putchar(c);
}

static inline int
abs(int n)
{
	if (n < 0)
		return -n;
	return n;
}

static int
print_int(int n)
{
	int ret = 0, i = 1;

	if (n > 0) {
		while (n/i >= 10)
			i *= 10;
	} else if (n < 0) {
		kputchar('-');
		ret++;
		while (n/i <= -10)
			i *= 10;
	}

	while (i > 0) {
		kputchar('0' + abs(n/i));
		ret++;
		n -= (n/i)*i;
		i /= 10;
	}

	return ret;
}

static int
print_ssize_t(ssize_t n)
{
	int ret = 0;
	ssize_t i = 1;

	if (n > 0) {
		while (n/i >= 10)
			i *= 10;
	} else if (n < 0) {
		kputchar('-');
		ret++;
		while (n/i <= -10)
			i *= 10;
	}

	while (i > 0) {
		kputchar('0' + abs(n/i));
		ret++;
		n -= (n/i)*i;
		i /= 10;
	}

	return ret;
}

static int
print_size_t(size_t n)
{
	int ret = 0;
	size_t i = 1;

	while (n/i >= 10)
		i *= 10;

	while (i > 0) {
		kputchar('0' + abs(n/i));
		ret++;
		n -= (n/i)*i;
		i /= 10;
	}

	return ret;
}

static int
print_hexint(uint_t n)
{
	uint_t x, mask;
	int i, bits, b, ret = 0;

	if (n == 0) {
		kputchar('0');
		return 1;
	} else {
		bits = sizeof(uint_t) * 8;
		b = bits;
		mask = 0xf << (bits - 4);

		for (i = 1; i <= b/4; i++) {
			if (n & mask)
				break;
			bits -= 4;
			mask >>= 4;
		}

		for (; i <= b/4; i++) {
			bits -= 4;
			x = (n & mask) >> bits;
			mask >>= 4;
			if (x < 0xa)
				kputchar('0' + x);
			else
				kputchar('a' + x - 10);
			ret++;
		}
	}

	return ret;
}

static int
print_pointer(uintptr_t p)
{
	uintptr_t x, mask;
	int i, bits, b, ret = 0;

	if (p == 0) {
		kputs("(nil)");
		return 5;
	} else {
		bits = sizeof(uintptr_t) * 8;
		b = bits;
		mask = 0xf << (bits - 4);

		kputs("0x");
		ret += 2;

		for (i = 1; i <= b/4; i++) {
			bits -= 4;
			x = (p & mask) >> bits;
			mask >>= 4;
			if (x < 0xa)
				kputchar('0' + x);
			else
				kputchar('a' + x - 10);
			ret++;
		}
	}

	return ret;
}

int
kprintf(const char *fmt, ...)
{
	va_list ap;
	int ret;

	va_start(ap, fmt);
	ret = kvprintf(fmt, ap);
	va_end(ap);

	return ret;
}

int
kvprintf(const char *fmt, va_list ap)
{
	int d, ret = 0;
	uint_t x;
	uintptr_t p;
	size_t sz;
	ssize_t ssz;
	char c, *s;

	while (*fmt) {
		switch (*fmt) {
		case '%':
			fmt++;
			switch (*fmt) {
			case 'd':
				d = va_arg(ap, int);
				ret += print_int(d);
				break;
			case 'x':
				x = va_arg(ap, uint_t);
				ret += print_hexint(x);
				break;
			case 'p':
				p = va_arg(ap, uintptr_t);
				ret += print_pointer(p);
				break;
			case 's':
				s = va_arg(ap, char*);
				ret += kputs(s);
				break;
			case 'c':
				c = va_arg(ap, char);
				kputchar(c);
				ret++;
				break;
			case 'z':
				fmt++;
				if (*fmt == 'u') {
					sz = va_arg(ap, size_t);
					ret += print_size_t(sz);
				} else if (*fmt == 'd') {
					ssz = va_arg(ap, ssize_t);
					ret += print_ssize_t(ssz);
				}
				break;
			case '%':
				kputchar('%');
				ret++;
			}
			break;
		default:
			kputchar(*fmt);
			ret++;
		}
		if (*fmt)
			fmt++;
	}

	return ret;
}
