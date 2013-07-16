#ifndef __ALLOC_H
#define __ALLOC_H

#include <inttypes.h>

void *kmalloc(size_t size);
void kfree(void *addr);
void *krealloc(void *addr, size_t size);
void *kcalloc(size_t nmemb, size_t size);
void kdump(void);

#endif	/* __ALLOC_H */
