#ifndef __MMU_H
#define __MMU_H

#define PAGE_SHIFT		12
#define PAGE_SIZE		(1UL << PAGE_SHIFT)
#define PAGE_MASK		(~(PAGE_SIZE - 1))
#define PAGE_ALIGN(x)		(((x) + PAGE_SIZE - 1) & PAGE_MASK)

#define L1_TYPE_MASK	3
#define L1_FAULT	0
#define L1_PAGE_TABLE	1
#define L1_SECTION	2

#define L2_TYPE_MASK	3
#define L2_PAGE_FAULT	0
#define L2_LARGE_PAGE	1
#define L2_SMALL_PAGE	2
#define L2_TINY_PAGE	3

#define PT_AP0		(1 << 4)
#define PT_AP1		(1 << 5)
#define PT_AP2		(1 << 9)


/* MMU access permissions
 * MMU_AP_X_Y
 * X = access permissions for PL1 or higher
 * Y = access permissions for PL0 (user mode)
 */
typedef enum {
	MMU_AP_RW_RW,
	MMU_AP_RW_RO,
	MMU_AP_RO_RO,
	MMU_AP_RW_NONE,
	MMU_AP_RO_NONE,
	MMU_AP_NONE_NONE
} mmu_ap_t;

void mmu_init();
void mmu_enable();
void mmu_disable();
int mmu_map_page(void *phys, void *virt, uint_t npages, mmu_ap_t perms);
int kmmap(void *virt, uint_t npages, mmu_ap_t perms);

#endif /* __MMU_H */
