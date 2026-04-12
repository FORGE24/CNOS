/* kernel/multiboot.h - Multiboot2 引导信息 (GRUB2) */

#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include <stdint.h>

struct vbe_mode_info;

#define MULTIBOOT2_BOOTLOADER_MAGIC 0x36d76289U

#define MULTIBOOT_TAG_TYPE_END 0
#define MULTIBOOT_TAG_TYPE_MMAP 6
#define MULTIBOOT_TAG_TYPE_FRAMEBUFFER 8

struct multiboot_tag {
    uint32_t type;
    uint32_t size;
} __attribute__((packed));

struct multiboot_tag_mmap {
    uint32_t type;
    uint32_t size;
    uint32_t entry_size;
    uint32_t entry_version;
} __attribute__((packed));

struct multiboot_mmap_entry {
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t reserved;
} __attribute__((packed));

struct multiboot_tag_framebuffer {
    uint32_t type;
    uint32_t size;
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t framebuffer_bpp;
    uint8_t framebuffer_type;
    uint8_t reserved0;
    uint8_t reserved1;
} __attribute__((packed));

#define MULTIBOOT_MEMORY_AVAILABLE 1

int multiboot_validate(uint32_t magic, uint64_t mbi_phys);
int multiboot_fill_vbe(uint64_t mbi_phys, struct vbe_mode_info *out);

#endif
