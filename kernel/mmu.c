#include <kernel.h>
#include <mmu.h>
#include <mm.h>

static u32 mmu_ttb[4096] __attribute__((__aligned__ (16 * 1024)));
static u32 l2[4096][256] __attribute__((__aligned__ (1024)));

void
mmu_init(void)
{
	int i;

	for (i = 0; i < 4096; i++)
		mmu_ttb[i] = L1_FAULT;

	asm volatile (
		/* invalidate TLB
		 * v1 is ignored */
		"mcr p15, 0, v1, c8, c7, 0	\n\t"
		/* completes the TLB invalidation */
		"dsb				\n\t"
		/* set TTBCR */
		"mov v1, #0			\n\t"
		"mcr p15, 0, v1, c2, c0, 2	\n\t"
		/* set TTBR0 */
		"ldr v1, =mmu_ttb		\n\t"
		"mcr p15, 0, v1, c2, c0, 0	\n\t"
		/* set DACR */
		"ldr v1, =0x55555555		\n\t"
		"mcr p15, 0, v1, c3, c0, 0	\n\t"
		/* make sure that SCTLR.AFE is disabled */
		"mrc p15, 0, v1, c1, c0, 0	\n\t"
		"bic v1, v1, #(1 << 29)		\n\t"
		"mcr p15, 0, v1, c1, c0, 0	\n\t"
		/* invalidate TLB */
		"mcr p15, 0, v1, c8, c7, 0	\n\t"
		/* completes the TLB invalidation */
		"dsb				\n\t"
		: : : "v1", "memory"
	);
}

void
mmu_enable(void)
{
	asm volatile (
		/* invalidate TLB */
		"mcr p15, 0, v1, c8, c7, 0	\n\t"
		/* completes the TLB invalidation */
		"dsb				\n\t"
		/* enable MMU */
		"mrc p15, 0, v1, c1, c0, 0	\n\t"
		"orr v1, v1, #1			\n\t"
		"mcr p15, 0, v1, c1, c0, 0	\n\t"
		: : : "v1"
	);
}

void
mmu_disable(void)
{
	asm volatile (
		/* disable MMU */
		"mrc p15, 0, v1, c1, c0, 0	\n\t"
		"bic v1, v1, #1			\n\t"
		"mcr p15, 0, v1, c1, c0, 0	\n\t"
		: : : "v1"
	);
}

uintptr_t
virt_to_phys(void *virt)
{
	uint_t pte_idx, pde_idx;
	u32 *pde;
	uintptr_t virt_a;

	virt_a = (uintptr_t)virt;
	pde_idx = virt_a >> 20;

	switch (mmu_ttb[pde_idx] & L1_TYPE_MASK) {
	case 1: /* page table */
		pde = (u32*)(mmu_ttb[pde_idx] & ~0x3ff);
		pte_idx = (virt_a & 0xff000) >> 12;
		if (pde[pte_idx] & L2_TYPE_MASK)
			return ((pde[pte_idx] & ~0xfff) | (virt_a & 0xfff));
		else
			return 0; /* not mapped */
	case 2: /* section */
		return ((mmu_ttb[pde_idx] & ~0xfffff) | (virt_a & 0xfffff));
	case 0: /* not mapped */
	default:
		return 0;
	}

	/* not mapped */
	return 0;
}

int
virt_is_mapped(void *virt)
{
	uint_t pte_idx, pde_idx;
	u32 *pde;
	uintptr_t virt_a;

	virt_a = (uintptr_t)virt;
	pde_idx = virt_a >> 20;

	switch (mmu_ttb[pde_idx] & L1_TYPE_MASK) {
	case 1: /* page table */
		pde = (u32*)(mmu_ttb[pde_idx] & ~0x3ff);
		pte_idx = (virt_a & 0xff000) >> 12;
		if (pde[pte_idx] & L2_TYPE_MASK)
			return 1;
		else
			return 0; /* fault */
	case 2: /* section */
		return 1;
	case 0: /* fault */
	default:
		return 0;
	}

	return 0;
}

/* map physical memory to virtual memory */
int
mmu_map_page(void *phys, void *virt, uint_t npages, mmu_ap_t perms)
{
	u32 pte, pte_perms;
	u32 *pde;
	uintptr_t phys_a, virt_a;
	uint_t i, pte_idx, pde_idx;

	phys_a = (uintptr_t)phys;
	virt_a = (uintptr_t)virt;

	if (npages == 0 || phys_a & (PAGE_SIZE - 1) || virt_a & (PAGE_SIZE - 1))
		return -EINVAL;

	switch (perms) {
	case MMU_AP_RW_RW:
		/* AP[2:0] = 011 */
		pte_perms = PT_AP0 | PT_AP1;
		break;
	case MMU_AP_RW_RO:
		/* AP[2:0] = 010 */
		pte_perms = PT_AP1;
		break;
	case MMU_AP_RO_RO:
		/* AP[2:0] = 111 */
		pte_perms = PT_AP2 | PT_AP1 | PT_AP0;
		break;
	case MMU_AP_RW_NONE:
		/* AP[2:0] = 001 */
		pte_perms = PT_AP0;
		break;
	case MMU_AP_RO_NONE:
		/* AP[2:0] = 101 */
		pte_perms = PT_AP2 | PT_AP0;
		break;
	case MMU_AP_NONE_NONE:
		/* AP[2:0] = 000 */
		pte_perms = 0;
		break;
	default:
		return -EINVAL;
	}

	for (i = 0; i < npages; i++) {
		pde_idx = virt_a >> 20;

		if (!mmu_ttb[pde_idx]) {
			int j;
			mmu_ttb[pde_idx] = (u32)(&l2[pde_idx]) | L1_PAGE_TABLE;
			for (j = 0; j < 256; j++)
				l2[pde_idx][j] = L2_PAGE_FAULT;
		}

		pde = (u32*)(mmu_ttb[pde_idx] & ~0x3ff);
		pte_idx = (virt_a & 0xff000) >> 12;
		pte = (phys_a & 0xfffff000) | L2_SMALL_PAGE;
		pde[pte_idx] = pte | pte_perms;

		phys_a += PAGE_SIZE;
		virt_a += PAGE_SIZE;
	}

	/* invalidate TLB */
	asm volatile("mcr p15, 0, v1, c8, c7, 0		\n\t"
		     "dsb				\n\t"
		     : : : "v1", "memory");

	return 0;
}

int
kmmap(void *virt, uint_t npages, mmu_ap_t perms)
{
	uint_t i;
	uintptr_t virt_a;
	void *pa;

	if (npages == 0)
		return -EINVAL;

	virt_a = (uintptr_t)virt;

	/* overflow */
	if (virt_a + npages * PAGE_SIZE < virt_a)
		return -EFAULT;

	for (i = 0; i < npages; i++) {
		if (virt_is_mapped((void*)virt_a)) {
			kprintf("WARNING: %p virtual address is already maped\n", virt);
			virt_a += PAGE_SIZE;
			continue;
		}
		pa = palloc(1);
		mmu_map_page(pa, (void*)virt_a, 1, perms);
		virt_a += PAGE_SIZE;
	}

	return 0;
}
