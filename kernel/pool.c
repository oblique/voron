#include <kernel.h>
#include <mmu.h>
#include <alloc.h>
#include <spinlock.h>
#include <pool.h>

struct arena {
	arena_id id;
	void *mem;
	size_t siz;
	int used;
};

struct pool {
	struct arena *arena;
	size_t siz;
};

static int ndebug = 0;
static spinlock_t pool_lock = SPINLOCK_INIT;

struct pool *
pool_init(void)
{
	struct pool *pool;

	pool = kmalloc(sizeof(struct pool));
	if (!pool)
		return NULL;
	pool->arena = NULL;
	pool->siz = 0;
	return pool;
}

void
pool_free(struct pool *pool)
{
	struct arena *arena;
	size_t i;

	spinlock_lock(&pool_lock);
	for (i = 0; i < pool->siz; i++) {
		arena = &pool->arena[i];
		kfree(arena->mem);
	}
	kfree(pool->arena);
	kfree(pool);
	spinlock_unlock(&pool_lock);
}

void *
arena_alloc(struct pool *pool, size_t n, arena_id id)
{
	struct arena *tmp;
	struct arena *arena;
	size_t i;

	spinlock_lock(&pool_lock);
	for (i = 0; i < pool->siz; i++) {
		arena = &pool->arena[i];
		if (arena->id == id) {
			if (ndebug > 0)
				kprintf("Found possible unused arena:%d\n", arena->id);
			if (!arena->used && arena->siz >= n) {
				if (ndebug > 0)
					kprintf("Found unused arena:%d of size %zu bytes\n",
					       arena->id, arena->siz);
				arena->used = 1;
				spinlock_unlock(&pool_lock);
				return arena->mem;
			}
		}
	}

	if (ndebug > 0)
		kprintf("Allocating new arena:%d of size %zu bytes\n",
		       id, n);

	tmp = krealloc(pool->arena, sizeof(struct arena) * (pool->siz + 1));
	if (!tmp)
		panic("out of memory");
	pool->arena = tmp;
	arena = &pool->arena[pool->siz];
	arena->id = id;
	arena->mem = kmalloc(n);
	if (!arena->mem)
		panic("out of memory");
	arena->siz = n;
	pool->siz++;
	spinlock_unlock(&pool_lock);
	return arena->mem;
}

void
arena_free(struct pool *pool, arena_id id)
{
	struct arena *arena;
	size_t i;

	spinlock_lock(&pool_lock);
	for (i = 0; i < pool->siz; i++) {
		arena = &pool->arena[i];
		if (arena->id == id)
			arena->used = 0;
	}
	spinlock_unlock(&pool_lock);
}
