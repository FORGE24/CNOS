/* kernel/vmm.h - 虚拟内存管理器 */

#ifndef VMM_H
#define VMM_H

#include <stdint.h>
#include <stddef.h>
#include "pmm.h"

/* 分页相关的位定义 */
#define PAGE_PRESENT (1ULL << 0)
#define PAGE_WRITE   (1ULL << 1)
#define PAGE_USER    (1ULL << 2)
#define PAGE_NX      (1ULL << 63)

/* 获取当前 PML4 地址 */
uint64_t vmm_get_current_pml4();

/* 初始化虚拟内存管理器 */
void vmm_init();

/* 映射虚拟地址到物理地址 */
void vmm_map(uint64_t pml4, uint64_t virt, uint64_t phys, uint64_t flags);

/* 取消映射虚拟地址 */
void vmm_unmap(uint64_t pml4, uint64_t virt);

/* 创建新的地址空间 (PML4) */
uint64_t vmm_create_address_space();

#endif
