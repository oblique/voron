#ifndef __PRINT_H
#define __PRINT_H

#include <varg.h>

int kputs(const char *s);
int kputchar(int c);
int kprintf(const char *fmt, ...);
int kvprintf(const char *fmt, va_list ap);

#endif	/* __PRINT_H */
