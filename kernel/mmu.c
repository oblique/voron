#include <inttypes.h>
#include <mmu.h>
#include <errno.h>
#include <debug.h>
#include <print.h>

static u32 mmu_ttb[4096] __attribute__((__aligned__ (16 * 1024)));
static u32 l2[4096][256] __attribute__((__aligned__ (1024)));

void mmu_init() {
	int i;

	for (i = 0; i < 4096; i++)
		mmu_ttb[i] = L1_FAULT;

	asm volatile (
		/* invalidate TLB
		 * v1 is ignored
		 */
		"mcr p15, 0, v1, c8, c7, 0	\n\t"
		/* set TTBCR */
		"mov v1, #0			\n\t"
		"mcr p15, 0, v1, c2, c0, 2	\n\t"
		/* set TTBR0 */
		"ldr v1, =mmu_ttb		\n\t"
		"mcr p15, 0, v1, c2, c0, 0	\n\t"
		/* set DACR */
		"ldr v1, =0x55555555		\n\t"
		"mcr p15, 0, v1, c3, c0, 0	\n\t"
		/* invalidate TLB */
		"mcr p15, 0, v1, c8, c7, 0	\n\t"
		/* enable AFE */
		"mrc p15, 0, v1, c1, c0, 0	\n\t"
		"orr v1, v1, #(1 << 29)		\n\t"
		"mcr p15, 0, v1, c1, c0, 0	\n\t"
		: : : "v1", "memory"
	);
}

void mmu_enable() {
	asm volatile (
		/* invalidate TLB */
		"mcr p15, 0, v1, c8, c7, 0	\n\t"
		/* enable MMU */
		"mrc p15, 0, v1, c1, c0, 0	\n\t"
		"orr v1, v1, #1			\n\t"
		"mcr p15, 0, v1, c1, c0, 0	\n\t"
		: : : "v1"
	);
}

void mmu_disable() {
	asm volatile (
		/* disable MMU */
		"mrc p15, 0, v1, c1, c0, 0	\n\t"
		"bic v1, v1, #1			\n\t"
		"mcr p15, 0, v1, c1, c0, 0	\n\t"
		: : : "v1"
	);
}

/* map physical memory to virtual memory */
int mmu_map_page(void *phys, void *virt, uint_t npages, mmu_ap_t perms) {
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
	asm volatile("mcr p15, 0, v1, c8, c7, 0"
		     : : : "v1", "memory");

	return 0;
}
