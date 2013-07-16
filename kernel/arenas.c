#include <kernel.h>
#include <mmu.h>
#include <alloc.h>
#include <arenas.h>

struct arena {
	arena_id id;
	void *mem;
	size_t siz;
	int used;
};

struct arenas {
	struct arena *arena;
	size_t siz;
};

static int ndebug = 0;

struct arenas *
arenas_init(void)
{
	struct arenas *arenas;

	arenas = kmalloc(sizeof(struct arenas));
	if (!arenas)
		return NULL;
	arenas->arena = NULL;
	arenas->siz = 0;
	return arenas;
}

void
arenas_free(struct arenas *arenas)
{
	struct arena *arena;
	size_t i;

	for (i = 0; i < arenas->siz; i++) {
		arena = &arenas->arena[i];
		kfree(arena->mem);
	}
	kfree(arenas->arena);
	kfree(arenas);
}

void *
arena_alloc(struct arenas *arenas, size_t n, arena_id id)
{
	struct arena *tmp;
	struct arena *arena;
	size_t i;

	for (i = 0; i < arenas->siz; i++) {
		arena = &arenas->arena[i];
		if (arena->id == id) {
			if (ndebug > 0)
				kprintf("Found possible unused arena:%d\n", arena->id);
			if (!arena->used && arena->siz >= n) {
				if (ndebug > 0)
					kprintf("Found unused arena:%d of size %zu bytes\n",
					       arena->id, arena->siz);
				arena->used = 1;
				return arena->mem;
			}
		}
	}

	if (ndebug > 0)
		kprintf("Allocating new arena:%d of size %zu bytes\n",
		       id, n);

	tmp = krealloc(arenas->arena, sizeof(struct arena) * (arenas->siz + 1));
	if (!tmp)
		panic("out of memory");
	arenas->arena = tmp;
	arena = &arenas->arena[arenas->siz];
	arena->id = id;
	arena->mem = kmalloc(n);
	if (!arena->mem)
		panic("out of memory");
	arena->siz = n;
	arenas->siz++;
	return arena->mem;
}

void
arena_free(struct arenas *arenas, arena_id id)
{
	struct arena *arena;
	size_t i;

	for (i = 0; i < arenas->siz; i++) {
		arena = &arenas->arena[i];
		if (arena->id == id)
			arena->used = 0;
	}
}
