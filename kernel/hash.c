#include <kernel.h>
#include <alloc.h>
#include <spinlock.h>
#include <hash.h>

enum { CHAIN_SIZ = 32 };

struct hentry {
	struct {
		void *data;
		size_t siz;
	} c[CHAIN_SIZ];
};

struct htable {
	struct hentry *e;
	struct hops *ops;
	size_t siz;
	spinlock_t lock;
};

struct htable *
init_htable(struct hops *ops, size_t siz)
{
	struct htable *ht;
	size_t i, j;

	if (!ops || !ops->hash || !ops->cmp || !siz)
		return NULL;

	ht = kmalloc(sizeof(*ht));
	if (!ht)
		return NULL;
	ht->e = kmalloc(siz * sizeof(*ht->e));
	if (!ht->e) {
		kfree(ht);
		return NULL;
	}
	for (i = 0; i < siz; i++) {
		for (j = 0; j < CHAIN_SIZ; j++) {
			ht->e[i].c[j].data = NULL;
			ht->e[i].c[j].siz = 0;
		}
	}
	ht->siz = siz;
	ht->ops = ops;
	spinlock_init(&ht->lock);
	return ht;
}

void
kfree_htable(struct htable *ht)
{
	kfree(ht->e);
	kfree(ht);
}

long
search_htable(struct htable *ht, void *data, size_t siz)
{
	struct hentry *e;
	struct hops *ops;
	long i;
	size_t j;

	if (!ht || !data || !siz)
		return -1;

	spinlock_lock(&ht->lock);
	ops = ht->ops;
	i = ops->hash(data, siz) % ht->siz;
	e = &ht->e[i];
	for (j = 0; j < CHAIN_SIZ; j++) {
		if (!e->c[j].data || e->c[j].siz != siz)
			continue;
		if (!ops->cmp(e->c[j].data, data, siz)) {
			spinlock_unlock(&ht->lock);
			return i;
		}
	}
	spinlock_unlock(&ht->lock);
	return -1;
}

long
insert_htable(struct htable *ht, void *data, size_t siz)
{
	struct hentry *e;
	struct hops *ops;
	long i;
	size_t j;

	if (!ht || !data || !siz)
		return -1;

	spinlock_lock(&ht->lock);
	ops = ht->ops;
	i = ops->hash(data, siz) % ht->siz;
	e = &ht->e[i];
	for (j = 0; j < CHAIN_SIZ; j++) {
		if (e->c[j].data)
			continue;
		e->c[j].data = data;
		e->c[j].siz = siz;
		spinlock_unlock(&ht->lock);
		return i;
	}
	spinlock_unlock(&ht->lock);
	return -1;
}

long
remove_htable(struct htable *ht, void *data, size_t siz)
{
	struct hentry *e;
	struct hops *ops;
	long i;
	size_t j;

	if (!ht || !data || !siz)
		return -1;

	spinlock_lock(&ht->lock);
	ops = ht->ops;
	i = ops->hash(data, siz) % ht->siz;
	e = &ht->e[i];
	for (j = 0; j < CHAIN_SIZ; j++) {
		if (!e->c[j].data || e->c[j].siz != siz)
			continue;
		if (!ops->cmp(e->c[j].data, data, siz)) {
			e->c[j].data = NULL;
			e->c[j].siz = 0;
			spinlock_unlock(&ht->lock);
			return i;
		}
	}
	spinlock_unlock(&ht->lock);
	return -1;
}
