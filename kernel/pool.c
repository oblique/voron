#include <kernel.h>
#include <mmu.h>
#include <alloc.h>
#include <spinlock.h>
#include <pool.h>

enum item_state {
	ITEM_UNUSED,
	ITEM_USED,
};

struct item {
	item_id id;
	void *mem;
	size_t siz;
	enum item_state state;
};

struct pool {
	struct item *item;
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
	pool->item = NULL;
	pool->siz = 0;
	return pool;
}

void
pool_free(struct pool *pool)
{
	struct item *item;
	size_t i;

	spinlock_lock(&pool_lock);
	for (i = 0; i < pool->siz; i++) {
		item = &pool->item[i];
		kfree(item->mem);
	}
	kfree(pool->item);
	kfree(pool);
	spinlock_unlock(&pool_lock);
}

/* Allocate an item in the pool of size `n' with `id'.
 * If there is an unused item of sufficient size with the same
 * `id' then it will be re-used. */
void *
item_alloc(struct pool *pool, size_t n, item_id id)
{
	struct item *tmp;
	struct item *item;
	size_t i;

	spinlock_lock(&pool_lock);
	for (i = 0; i < pool->siz; i++) {
		item = &pool->item[i];
		if (item->id == id) {
			if (ndebug > 0)
				kprintf("Found possible unused item:%d\n", item->id);
			if (item->state == ITEM_UNUSED && item->siz >= n) {
				if (ndebug > 0)
					kprintf("Found unused item:%d of size %zu bytes\n",
						item->id, item->siz);
				item->state = ITEM_USED;
				spinlock_unlock(&pool_lock);
				return item->mem;
			}
		}
	}

	if (ndebug > 0)
		kprintf("Allocating new item:%d of size %zu bytes\n",
			id, n);

	tmp = krealloc(pool->item, sizeof(struct item) * (pool->siz + 1));
	if (!tmp)
		panic("out of memory");
	pool->item = tmp;
	item = &pool->item[pool->siz];
	item->id = id;
	item->mem = kmalloc(n);
	if (!item->mem)
		panic("out of memory");
	item->siz = n;
	item->state = ITEM_USED;
	pool->siz++;
	spinlock_unlock(&pool_lock);
	return item->mem;
}

/* Free all items with `id'.  This does not actually
 * free the underlying memory, it just marks those items
 * as unused. */
void
item_free(struct pool *pool, item_id id)
{
	struct item *item;
	size_t i;

	spinlock_lock(&pool_lock);
	for (i = 0; i < pool->siz; i++) {
		item = &pool->item[i];
		if (item->id == id)
			item->state= ITEM_UNUSED;
	}
	spinlock_unlock(&pool_lock);
}
