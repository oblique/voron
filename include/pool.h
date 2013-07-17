#ifndef __POOL_H
#define __POOL_H

typedef int arena_id;

struct pool *pool_init(void);
void pool_free(struct pool *pool);
void *arena_alloc(struct pool *pool, size_t n, arena_id id);
void arena_free(struct pool *pool, arena_id id);

#endif	/* __POOL_H */
