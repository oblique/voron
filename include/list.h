#ifndef __LIST_H
#define __LIST_H

#include <kernel.h>

struct list_head {
	struct list_head *prev;
	struct list_head *next;
};

#define LIST_HEAD_INIT(name)	{ &(name), &(name) }
#define LIST_HEAD(name)	struct list_head name = LIST_HEAD_INIT(name)

#define list_entry(ptr, type, member) container_of(ptr, type, member)

/* list_for_each - iterate over a list
 * pos: the &struct list_head to use as a loop cursor.
 * head: the head for your list.
 */
#define list_for_each(pos, head)		\
	for (pos = (head)->next; pos != (head); \
	     pos = pos->next)

/* list_for_each_safe - iterate over a list safe against removal of list entry
 * pos: the &struct list_head to use as a loop cursor.
 * n: another &struct list_head to use as temporary storage
 * head: the head for your list.
 */
#define list_for_each_safe(pos, n, head)			\
	for (pos = (head)->next, n = pos->next; pos != (head);	\
	     pos = n, n = pos->next)

static inline void INIT_LIST_HEAD(struct list_head *list) {
	list->prev = list;
	list->next = list;
}

/* Note: list_empty() on entry does not return true after this, the entry is
 * in an undefined state.
 */
static inline void list_del(struct list_head *entry) {
	entry->prev->next = entry->next;
	entry->next->prev = entry->prev;
	entry->next = NULL;
	entry->prev = NULL;
}

static inline void list_add(struct list_head *entry, struct list_head *head) {
	head->next->prev = entry;
	entry->next = head->next;
	entry->prev = head;
	head->next = entry;
}

static inline void list_add_tail(struct list_head *entry, struct list_head *head) {
	head->prev->next = entry;
	entry->next = head;
	entry->prev = head->prev;
	head->prev = entry;
}

static inline int list_empty(struct list_head *head) {
	return head->next == head;
}


#endif	/* __LIST_H */
