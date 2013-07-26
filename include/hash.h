#ifndef __HASH_H
#define __HASH_H

#include <inttypes.h>

struct hops {
        long (*hash)(void *data, size_t siz);
        int (*cmp)(void *src, void *dst, size_t siz);
};

struct htable *init_htable(struct hops *ops, size_t siz);
void free_htable(struct htable *ht);
long search_htable(struct htable *ht, void *data, size_t siz);
long insert_htable(struct htable *ht, void *data, size_t siz);
long remove_htable(struct htable *ht, void *data, size_t siz);

#endif /* __HASH_H */
