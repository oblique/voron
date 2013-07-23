/* Simple hash table implementation using open-addressing */
#include <kernel.h>
#include <mmu.h>
#include <alloc.h>
#include <spinlock.h>
#include <hash.h>

enum hent_state {
	UNUSED,
	USED,
	REMOVED,
};

struct hent {
	void *data;
	size_t siz;
	enum hent_state state;
};

struct htable {
	struct hent *hent;
	struct hent_ops *hent_ops;
	size_t siz;
	spinlock_t lock;
};

struct htable *
init_htable(struct hent_ops *hent_ops, size_t n)
{
	struct htable *htable;
	size_t i;

	htable = kmalloc(sizeof(*htable));
	if (!htable)
		return NULL;
	htable->hent = kcalloc(n, sizeof(struct hent));
	if (!htable->hent) {
		kfree(htable);
		return NULL;
	}
	for (i = 0; i < n; i++)
		htable->hent[i].state = UNUSED;
	htable->siz = n;
	htable->hent_ops = hent_ops;
	spinlock_init(&htable->lock);
	return htable;
}

void
free_htable(struct htable *htable)
{
	kfree(htable->hent);
	kfree(htable);
}

/* Search the hash table for the given entry.  Returns -1
 * on failure and the index of the entry on success. */
int
search_htable(struct htable *htable, void *data, size_t siz)
{
	struct hent *hent;
	struct hent_ops *ops;
	size_t idx, i, j;

	spinlock_lock(&htable->lock);
	ops = htable->hent_ops;
	idx = ops->hash(data, siz) % htable->siz;
	j = idx;
	for (i = 0; i < htable->siz; i++) {
		hent = &htable->hent[j];
		if (hent->state == UNUSED) {
			spinlock_unlock(&htable->lock);
			return -1;
		}
		if (hent->state == USED && !ops->cmp(hent->data, data, siz)) {
			spinlock_unlock(&htable->lock);
			return j;
		}
		j++;
		j %= htable->siz;
	}
	spinlock_unlock(&htable->lock);
	return -1;
}

/* Insert the given entry in the hash table.  Ignore duplicates.
 * Return the index on a successful insertion, -1 if the table is full. */
int
insert_htable(struct htable *htable, void *data, size_t siz)
{
	struct hent *hent;
	struct hent_ops *ops;
	size_t idx, i, j;

	spinlock_lock(&htable->lock);
	ops = htable->hent_ops;
	idx = ops->hash(data, siz) % htable->siz;
	j = idx;
	for (i = 0; i < htable->siz; i++) {
		hent = &htable->hent[j];
		if (hent->state == USED && !ops->cmp(hent->data, data, siz)) {
			spinlock_unlock(&htable->lock);
			return j;
		}
		if (hent->state == UNUSED || hent->state == REMOVED) {
			hent->data = data;
			hent->siz = siz;
			hent->state = USED;
			spinlock_unlock(&htable->lock);
			return j;
		}
		j++;
		j %= htable->siz;
	}
	spinlock_unlock(&htable->lock);
	return -1;
}

/* Remove an entry from the hash table.  Return -1 on failure,
 * and the index of the entry on success. */
int
remove_htable(struct htable *htable, void *data, size_t siz)
{
	struct hent *hent;
	struct hent_ops *ops;
	size_t idx, i, j;

	spinlock_lock(&htable->lock);
	ops = htable->hent_ops;
	idx = ops->hash(data, siz) % htable->siz;
	j = idx;
	for (i = 0; i < htable->siz; i++) {
		hent = &htable->hent[j];
		if (hent->state == UNUSED) {
			spinlock_unlock(&htable->lock);
			return -1;
		}
		if (hent->state == USED && !ops->cmp(hent->data, data, siz)) {
			hent->state = REMOVED;
			spinlock_unlock(&htable->lock);
			return j;
		}
		j++;
		j %= htable->siz;
	}
	spinlock_unlock(&htable->lock);
	return -1;
}
