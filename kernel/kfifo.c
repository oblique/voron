#include <kernel.h>
#include <alloc.h>
#include <spinlock.h>
#include "kfifo.h"

struct kfifo {
	uint8_t *buf;
	size_t siz;
	size_t cap;
};

static spinlock_t kfifo_lock = SPINLOCK_INIT;

struct kfifo *
init_kfifo(size_t cap)
{
	struct kfifo *kfifo;

	kfifo = kmalloc(sizeof(*kfifo));
	if (!kfifo)
		return NULL;
	if (cap > 0) {
		kfifo->buf = kmalloc(cap);
		if (!kfifo->buf) {
			kfree(kfifo);
			return NULL;
		}
	}
	kfifo->siz = 0;
	kfifo->cap = cap;
	return kfifo;
}

void
free_kfifo(struct kfifo *kfifo)
{
	spinlock_lock(&kfifo_lock);
	kfree(kfifo->buf);
	kfree(kfifo);
	spinlock_unlock(&kfifo_lock);
}

int
enqueue_kfifo(struct kfifo *kfifo, void *buf, size_t siz)
{
	void *tmp;

	spinlock_lock(&kfifo_lock);
	if (kfifo->cap - kfifo->siz >= siz) {
		memcpy(kfifo->buf + kfifo->siz, buf, siz);
		kfifo->siz += siz;
		spinlock_unlock(&kfifo_lock);
		return 0;
	}

	tmp = krealloc(kfifo->buf, kfifo->cap + siz);
	if (!tmp) {
		spinlock_unlock(&kfifo_lock);
		return -1;
	}
	kfifo->buf = tmp;
	memcpy(kfifo->buf + kfifo->siz, buf, siz);
	kfifo->siz += siz;
	kfifo->cap += siz;
	spinlock_unlock(&kfifo_lock);
	return 0;
}

int
dequeue_kfifo(struct kfifo *kfifo, void *buf, size_t siz)
{
	spinlock_lock(&kfifo_lock);
	if (kfifo->siz < siz) {
		spinlock_unlock(&kfifo_lock);
		return -1;
	}
	memcpy(buf, kfifo->buf, siz);
	memmove(kfifo->buf, kfifo->buf + siz, kfifo->siz - siz);
	kfifo->siz -= siz;
	spinlock_unlock(&kfifo_lock);
	return 0;
}
