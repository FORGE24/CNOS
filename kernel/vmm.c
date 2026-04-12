/* kernel/vmm.c - 虚拟内存管理器实现 */

#include "vmm.h"
#include <stdint.h>
#include <stddef.h>
#include "pmm.h"

/* 辅助函数：从 CR3 寄存器读取当前 PML4 地址 */
uint64_t vmm_get_current_pml4() {
    uint64_t pml4;
    __asm__ volatile("mov %%cr3, %0" : "=r"(pml4));
    return pml4;
}

/* 辅助函数：将 PML4 地址加载到 CR3 寄存器以切换地址空间 */
static inline void vmm_set_pml4(uint64_t pml4) {
    __asm__ volatile("mov %0, %%cr3" :: "r"(pml4));
}

/* 初始化虚拟内存管理器 */
void vmm_init() {
    /* 
     * 初始时，我们可以继续使用 entry.asm 创建的页表。
     * 后续我们可以创建一个专门的内核页表。
     */
}

/* 在 PML4/PDPT/PD/PT 中获取条目索引的宏 */
#define PML4_INDEX(addr) (((addr) >> 39) & 0x1FF)
#define PDPT_INDEX(addr) (((addr) >> 30) & 0x1FF)
#define PD_INDEX(addr)   (((addr) >> 21) & 0x1FF)
#define PT_INDEX(addr)   (((addr) >> 12) & 0x1FF)

/* 获取或创建一个下一级页表 */
static uint64_t *get_next_level(uint64_t *current_level, uint64_t index, uint64_t flags) {
    if (!(current_level[index] & PAGE_PRESENT)) {
        void *new_page = pmm_alloc_page();
        if (!new_page) return NULL;
        
        /* 清空新分配的页表页 */
        uint8_t *ptr = (uint8_t *)new_page;
        for (int i = 0; i < PAGE_SIZE; i++) ptr[i] = 0;

        current_level[index] = (uint64_t)new_page | flags | PAGE_PRESENT;
    }
    /* 条目低 12 位是属性位，其余位是物理地址 (4KB 对齐) */
    return (uint64_t *)(current_level[index] & ~0xFFFULL);
}

/* 映射虚拟地址到物理地址 */
void vmm_map(uint64_t pml4_phys, uint64_t virt, uint64_t phys, uint64_t flags) {
    uint64_t *pml4 = (uint64_t *)pml4_phys;
    
    uint64_t *pdpt = get_next_level(pml4, PML4_INDEX(virt), flags);
    if (!pdpt) return;
    
    uint64_t *pd   = get_next_level(pdpt, PDPT_INDEX(virt), flags);
    if (!pd) return;
    
    uint64_t *pt   = get_next_level(pd, PD_INDEX(virt), flags);
    if (!pt) return;
    
    pt[PT_INDEX(virt)] = (phys & ~0xFFFULL) | flags | PAGE_PRESENT;
}

void vmm_unmap(uint64_t pml4_phys, uint64_t virt) {
    uint64_t *pml4 = (uint64_t *)pml4_phys;
    
    if (!(pml4[PML4_INDEX(virt)] & PAGE_PRESENT)) return;
    uint64_t *pdpt = (uint64_t *)(pml4[PML4_INDEX(virt)] & ~0xFFFULL);
    
    if (!(pdpt[PDPT_INDEX(virt)] & PAGE_PRESENT)) return;
    uint64_t *pd = (uint64_t *)(pdpt[PDPT_INDEX(virt)] & ~0xFFFULL);
    
    if (!(pd[PD_INDEX(virt)] & PAGE_PRESENT)) return;
    uint64_t *pt = (uint64_t *)(pd[PD_INDEX(virt)] & ~0xFFFULL);
    
    pt[PT_INDEX(virt)] = 0;
    
    /* 刷新 TLB */
    __asm__ volatile("invlpg (%0)" :: "r"(virt) : "memory");
}

uint64_t vmm_create_address_space() {
    void *new_pml4_page = pmm_alloc_page();
    if (!new_pml4_page) return 0;
    
    uint64_t *new_pml4 = (uint64_t *)new_pml4_page;
    uint64_t *current_pml4 = (uint64_t *)vmm_get_current_pml4();
    
    /* 
     * 映射内核空间：
     * 通常我们将内核映射在所有进程的高地址空间 (Shared)。
     * 这里简化起见，我们复制当前 PML4 的所有条目。
     */
    for (int i = 0; i < 512; i++) {
        new_pml4[i] = current_pml4[i];
    }
    
    return (uint64_t)new_pml4;
}
