#ifndef __POOL_H
#define __POOL_H

#include <inttypes.h>

typedef int item_id;

struct pool *init_pool(void);
void free_pool(struct pool *pool);
void *alloc_item(struct pool *pool, size_t n, item_id id);
void free_items(struct pool *pool, item_id id);

#endif	/* __POOL_H */
