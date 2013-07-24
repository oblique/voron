#include <kernel.h>
#include <alloc.h>
#include <spinlock.h>
#include <kfifo.h>

struct kfifo {
	uint8_t *buf;
	size_t siz;
	size_t cap;
	spinlock_t lock;
};

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
	spinlock_init(&kfifo->lock);
	return kfifo;
}

void
free_kfifo(struct kfifo *kfifo)
{
	kfree(kfifo->buf);
	kfree(kfifo);
}

int
enqueue_kfifo(struct kfifo *kfifo, const void *buf, size_t siz)
{
	void *tmp;

	if (!siz)
		return 0;

	spinlock_lock(&kfifo->lock);
	if (kfifo->cap - kfifo->siz >= siz) {
		memcpy(kfifo->buf + kfifo->siz, buf, siz);
		kfifo->siz += siz;
		spinlock_unlock(&kfifo->lock);
		return 0;
	}

	tmp = krealloc(kfifo->buf, kfifo->cap + siz);
	if (!tmp) {
		spinlock_unlock(&kfifo->lock);
		return -ENOMEM;
	}
	kfifo->buf = tmp;
	memcpy(kfifo->buf + kfifo->siz, buf, siz);
	kfifo->siz += siz;
	kfifo->cap += siz;
	spinlock_unlock(&kfifo->lock);
	return 0;
}

int
dequeue_kfifo(struct kfifo *kfifo, void *buf, size_t siz)
{
	if (!siz)
		return 0;

	spinlock_lock(&kfifo->lock);
	if (kfifo->siz < siz) {
		spinlock_unlock(&kfifo->lock);
		return -EINVAL;
	}
	memcpy(buf, kfifo->buf, siz);
	memmove(kfifo->buf, kfifo->buf + siz, kfifo->siz - siz);
	kfifo->siz -= siz;
	spinlock_unlock(&kfifo->lock);
	return 0;
}
