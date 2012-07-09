#ifndef __VARG_H
#define __VARG_H

#include <inttypes.h>

#define __BND			(sizeof(uintptr_t) - 1)
#define __bnd(X)		((sizeof(X) + __BND) & (~__BND))

/* AAPCS p.28 */
struct __va_list {
	char *__ap;
};

typedef struct __va_list va_list;

#define va_start(ap, last)	((void) ((ap).__ap = ((char*) &(last) + __bnd(last))))
#define va_end(ap)		((void) 0)
#define va_arg(ap, type)	(*(type*) (((ap).__ap += __bnd(type)) - __bnd(type)))
#define va_copy(dest, src)	((void) ((dest).__ap = (src).__ap))

#endif	/* __VARG_H */
