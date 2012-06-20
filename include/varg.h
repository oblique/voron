#ifndef __VARG_H
#define __VARG_H

#include <inttypes.h>

#define _BND			(sizeof(uintptr_t) - 1)
#define _bnd(X)		((sizeof(X) + _BND) & (~_BND))

/* AAPCS p.28 */
struct __va_list {
	char *__ap;
};

typedef struct __va_list va_list;

#define va_start(ap, last)	((void) ((ap).__ap = ((char*) &(last) + _bnd(last))))
#define va_end(ap)		((void) 0)
#define va_arg(ap, type)	(*(type*) (((ap).__ap += _bnd(type)) - _bnd(type)))
#define va_copy(dest, src)	((void) ((dest).__ap = (src).__ap))

#endif	/* __VARG_H */
