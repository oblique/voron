#include <kernel.h>
#include <list.h>
#include <spinlock.h>
#include <mmu.h>
#include <alloc.h>

struct mem_chunk {
	union {
		void *start;
		uintptr_t start_a;
	};
	size_t size;
	struct list_head list;
};

#define get_mem_chunk(addr) (((struct mem_chunk*)(addr)) - 1)

static struct list_head freelist;
static struct list_head alloclist;
static spinlock_t freelist_lock = SPINLOCK_INIT;
static spinlock_t alloclist_lock = SPINLOCK_INIT;
static void *heap_last;

extern void *_kernel_heap_start;
extern void *_kernel_heap_end;

static size_t
roundup(size_t size)
{
	size_t ret;

	if (size <= 16)
		return 16;

	/* get the number of leading zeros */
	asm volatile("clz %0, %1" : "=r" (ret) : "r" (size));

	/* round up to the next 'power of 2' number */
	ret = 1 << (32 - ret);

	if (size != (ret >> 1))
		return ret;
	else
		return ret >> 1;
}

void *
kmalloc(size_t size)
{
	int ret;
	uint_t npages;
	uintptr_t heap_last_a, tmp_addr;
	struct mem_chunk *memc, *tmp_mc;
	struct list_head *pos;

	size = roundup(size);
	heap_last_a = (uintptr_t)heap_last;

	spinlock_lock(&freelist_lock);
	list_for_each(pos, &freelist) {
		memc = list_entry(pos, struct mem_chunk, list);
		if (memc->size == size) {
			list_del(&memc->list);
			spinlock_unlock(&freelist_lock);
			spinlock_lock(&alloclist_lock);
			list_add(&memc->list, &alloclist);
			spinlock_unlock(&alloclist_lock);
			return memc->start;
		}
	}
	spinlock_unlock(&freelist_lock);

	npages = PAGE_ALIGN(sizeof(struct mem_chunk) + size) >> PAGE_SHIFT;

	if (heap_last_a + npages * PAGE_SIZE > (uintptr_t)&_kernel_heap_end)
		return NULL;

	ret = kmmap(heap_last, npages, MMU_AP_RW_NONE);
	if (ret < 0)
		return NULL;

	memc = heap_last;
	memc->start_a = sizeof(struct mem_chunk) + heap_last_a;
	memc->size = size;

	spinlock_lock(&alloclist_lock);
	list_add(&memc->list, &alloclist);
	spinlock_unlock(&alloclist_lock);

	heap_last_a += npages * PAGE_SIZE;
	heap_last = (void*)heap_last_a;

	tmp_addr = memc->start_a + memc->size;
	while (tmp_addr + sizeof(struct mem_chunk) + memc->size < heap_last_a) {
		tmp_mc = (void*)tmp_addr;
		tmp_mc->start_a = sizeof(struct mem_chunk) + tmp_addr;
		tmp_mc->size = memc->size;
		tmp_addr += sizeof(struct mem_chunk) + tmp_mc->size;
		spinlock_lock(&freelist_lock);
		list_add(&tmp_mc->list, &freelist);
		spinlock_unlock(&freelist_lock);
	}

	return memc->start;
}

void
kfree(void *addr)
{
	struct mem_chunk *memc, *tmp;
	struct list_head *pos;

	memc = get_mem_chunk(addr);

	spinlock_lock(&alloclist_lock);
	list_for_each(pos, &alloclist) {
		tmp = list_entry(pos, struct mem_chunk, list);
		if (tmp == memc) {
			list_del(&memc->list);
			spinlock_lock(&freelist_lock);
			list_add(&memc->list, &freelist);
			spinlock_unlock(&freelist_lock);
			break;
		}
	}
	spinlock_unlock(&alloclist_lock);
}

void *
krealloc(void *addr, size_t size)
{
	void *p;
	struct list_head *pos;
	struct mem_chunk *memc;

	if (!size && addr) {
		kfree(addr);
		return NULL;
	}

	if (addr) {
		/* Lookup for the old base pointer `addr' if it is part of a
		 * previous allocation */
		spinlock_lock(&alloclist_lock);
		list_for_each(pos, &alloclist) {
			memc = list_entry(pos, struct mem_chunk, list);
			if (addr == memc->start)
				goto found;
		}
		spinlock_unlock(&alloclist_lock);
	}

	/* Allocate new space at a different base address */
	p = kmalloc(size);
	if (!p)
		return NULL;

	return p;
found:
	/* Are we shrinking the space?  If so just return the old `addr' */
	if (size <= memc->size) {
		spinlock_unlock(&alloclist_lock);
		return addr;
	}
	spinlock_unlock(&alloclist_lock);

	/* Allocate some space, copy over the old contents at `addr' and
	 * free that space */
	p = kmalloc(size);
	if (!p)
		return NULL;

	memcpy(p, addr, memc->size);
	kfree(addr);

	return p;
}

void *
kcalloc(size_t nmemb, size_t size)
{
	void *p;

	p = kmalloc(size * nmemb);
	if (!p)
		return NULL;
	memset(p, 0, size * nmemb);
	return p;
}

void
kdump(void)
{
	struct list_head *pos, *tmp;
	struct mem_chunk *memc;

	kprintf("alloc list\n");
	list_for_each_safe(pos, tmp, &alloclist) {
		memc = list_entry(pos, struct mem_chunk, list);
		kprintf("%p (phys: %p): %p %p %d\n", pos, virt_to_phys(pos), memc, memc->start, memc->size);
	}

	kprintf("\nfree list\n");
	list_for_each_safe(pos, tmp, &freelist) {
		memc = list_entry(pos, struct mem_chunk, list);
		kprintf("%p (phys: %p): %p %p %d\n", pos, virt_to_phys(pos), memc, memc->start, memc->size);
	}
}

__attribute__ ((__constructor__))
static void
alloc_init(void)
{
	INIT_LIST_HEAD(&freelist);
	INIT_LIST_HEAD(&alloclist);
	heap_last = &_kernel_heap_start;
}
