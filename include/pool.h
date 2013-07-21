#ifndef __POOL_H
#define __POOL_H

typedef int item_id;

struct pool *init_pool(void);
void free_pool(struct pool *pool);
void *alloc_item(struct pool *pool, size_t n, item_id id);
void free_item(struct pool *pool, item_id id);

#endif	/* __POOL_H */
