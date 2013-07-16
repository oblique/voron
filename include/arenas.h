#ifndef __ARENAS_H
#define __ARENAS_H

typedef int arena_id;

struct arenas *arenas_init(void);
void arenas_free(struct arenas *arenas);
void *arena_alloc(struct arenas *arenas, size_t n, arena_id id);
void arena_free(struct arenas *arenas, arena_id id);

#endif	/* __ARENAS_H */
