#ifndef __STDDEF_H
#define __STDDEF_H

#include <inttypes.h>

#define NULL	((void*)0)

enum {
	false = 0,
	true = 1
};

#define offsetof(type, member) ((uintptr_t)&((type*)0)->member)

#endif	/* __STDDEF_H */
