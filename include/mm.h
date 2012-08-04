#ifndef __MM_H
#define __MM_H

#include <inttypes.h>

void *palloc(uint_t npages);
int pfree(void *paddr, uint_t npages);

#endif /* __MM_H */
