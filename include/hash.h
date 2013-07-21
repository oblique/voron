#ifndef __HASH_H
#define __HASH_H

struct hent_ops {
	unsigned long (*hash)(void *data, size_t siz);
	int (*cmp)(void *src, void *dst, size_t siz);
};

struct htable *init_htable(struct hent_ops *hent_ops, size_t n);
void free_htable(struct htable *htable);
int search_htable(struct htable *htable, void *data, size_t siz);
int insert_htable(struct htable *htable, void *data, size_t siz);
int remove_htable(struct htable *htable, void *data, size_t siz);

#endif /* __HASH_H */
