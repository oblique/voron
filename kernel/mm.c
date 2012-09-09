#include <kernel.h>
#include <mmu.h>

#define ALIGN32(x)	(((x) + 31) & ~31)

extern void *_ram_start;
extern void *_ram_end;
extern void *_kernel_bin_start;
extern void *_kernel_bin_end;
extern void *_kernel_heap_start;
extern void *_kernel_heap_end;

static u32 *bitmap;
static uint_t bitmapsz;
static uint_t kbound_s;
static uint_t kbound_e;
static uint_t kheapbound_s;
static uint_t kheapbound_e;

/* set range of bits */
static int set_bitmap_bits(uint_t start_bit, uint_t n, int flag) {
	uint_t bound_s, bound_e, i;
	u32 bits;

	if (n == 0)
		return -EINVAL;

	bound_s = start_bit;
	bound_e = start_bit + n;

	if (bound_e > bitmapsz * 32)
		return -EINVAL;

	if (bound_s / 32 != bound_e / 32) {
		bits = ~((1 << (bound_s % 32)) - 1);
		if (flag)
			bitmap[bound_s/32] |= bits;
		else
			bitmap[bound_s/32] &= ~bits;

		bits = (1 << (bound_e % 32)) - 1;
		if (flag)
			bitmap[bound_e/32] |= bits;
		else
			bitmap[bound_e/32] &= ~bits;
	} else {
		bits = ~((1 << (bound_s % 32)) - 1);
		bits &= (1 << (bound_e % 32)) - 1;
		if (flag)
			bitmap[bound_s/32] |= bits;
		else
			bitmap[bound_s/32] &= ~bits;
	}

	for (i = (bound_s / 32) + 1; i < bound_e / 32; i++) {
		if (flag)
			bitmap[i] = ~0U;
		else
			bitmap[i] = 0;
	}

	return 0;
}

void mm_init() {
	uint_t i;
	size_t ramsz;

	kbound_s = PAGE_ALIGN(PTR_DIFF(&_kernel_bin_start, &_ram_start)) >> PAGE_SHIFT;
	kbound_e = PAGE_ALIGN(PTR_DIFF(&_kernel_bin_end, &_ram_start)) >> PAGE_SHIFT;
	kheapbound_s = PAGE_ALIGN(PTR_DIFF(&_kernel_heap_start, &_ram_start)) >> PAGE_SHIFT;
	kheapbound_e = PAGE_ALIGN(PTR_DIFF(&_kernel_heap_end, &_ram_start)) >> PAGE_SHIFT;

	ramsz = PTR_DIFF(&_ram_end, &_ram_start) + 1;
	bitmapsz = ALIGN32(ramsz >> PAGE_SHIFT) / 32;
	bitmap = (u32*)&_kernel_bin_end;
	kbound_e += PAGE_ALIGN(bitmapsz * 4) >> PAGE_SHIFT;

	for (i = 0; i < bitmapsz; i++)
		bitmap[i] = 0;

	set_bitmap_bits(kbound_s, kheapbound_s - kbound_s, 1);
	set_bitmap_bits(kheapbound_e, kbound_e - kheapbound_e, 1);

	mmu_init();

	/* map on-chip memory */
	mmu_map_page((void*)0x40000000, (void*)0x40000000,
		     0x40000, MMU_AP_RW_NONE);
	/* map kernel memory */
	mmu_map_page(&_kernel_bin_start, &_kernel_bin_start,
		     kheapbound_s - kbound_s, MMU_AP_RW_NONE);
	mmu_map_page(&_kernel_heap_end, &_kernel_heap_end,
		     kbound_e - kheapbound_e, MMU_AP_RW_NONE);

	mmu_enable();
}

void *palloc(uint_t npages) {
	uint_t i, j, n;
	uint_t sbit;
	int ret;

	if (npages == 0)
		return ERR_PTR(-EINVAL);

	n = 0;
	for (i = 0; i < bitmapsz; i++) {
		/* if bitmap[i] has unallocated page */
		if (bitmap[i] != ~0U) {
			for (j = 0; j < 32; j++) {
				if (n > 0 && (bitmap[i] & (1 << j)))
					n = 0;
				else if (!(bitmap[i] & (1 << j)))
					n++;
				if (n == npages) {
					sbit = i * 32 + j - n;
					goto out;
				}
			}
		} else
			n = 0;
	}

	return ERR_PTR(-EINVAL);

out:
	ret = set_bitmap_bits(sbit, npages, 1);
	if (ret < 0)
		return ERR_PTR(ret);
	return (void*)((uintptr_t)sbit * PAGE_SIZE + (uintptr_t)&_ram_start);
}

int pfree(void *paddr, uint_t npages) {
	uintptr_t paddr_a = (uintptr_t)paddr;
	uint_t sbit;

	if (npages == 0 || paddr_a & (PAGE_SIZE - 1) ||
	    paddr_a < (uintptr_t)&_ram_start ||
	    paddr_a >= (uintptr_t)&_ram_end)
		return -EINVAL;

	sbit = (paddr_a - (uintptr_t)&_ram_start) >> PAGE_SHIFT;

	/* we cannot free the memory of ourself */
	if ((sbit >= kbound_s && sbit < kheapbound_s) ||
	    (sbit >= kheapbound_e && sbit < kbound_e))
		return -EINVAL;

	return set_bitmap_bits(sbit, npages, 0);
}
