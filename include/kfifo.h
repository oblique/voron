#ifndef __KFIFO_H
#define __KFIFO_H

#include <inttypes.h>

struct kfifo *init_kfifo(size_t cap);
void free_kfifo(struct kfifo *kfifo);
int enqueue_kfifo(struct kfifo *kfifo, void *data, size_t siz);
int dequeue_kfifo(struct kfifo *kfifo, void *data, size_t siz);

#endif /* __KFIFO_H */
