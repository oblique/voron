#ifndef __KERNEL_H
#define __KERNEL_H

#include <inttypes.h>
#include <stddef.h>
#include <string.h>
#include <print.h>
#include <io.h>
#include <errno.h>
#include <varg.h>
#include <regs.h>
#include <alloc.h>
#include <debug.h>
#include <atomic.h>

#define __unused	__attribute__((__unused__))

#define container_of(ptr, type, member) ({		\
	const typeof(((type*)0)->member) *__mptr = (ptr);	\
	(type*)((uintptr_t)__mptr - offsetof(type, member));	\
})

#define ARRAY_SIZE(arr)	(sizeof(arr)/sizeof(arr[0]))

#endif	/* __KERNEL_H */
