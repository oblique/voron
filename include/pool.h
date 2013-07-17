#ifndef __POOL_H
#define __POOL_H

typedef int item_id;

struct pool *pool_init(void);
void pool_free(struct pool *pool);
void *item_alloc(struct pool *pool, size_t n, item_id id);
void item_free(struct pool *pool, item_id id);

#endif	/* __POOL_H */
