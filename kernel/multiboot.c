/* kernel/multiboot.c - 解析 GRUB2 传递的 Multiboot2 信息 */

#include "multiboot.h"
#include <stddef.h>
#include <stdint.h>

static struct multiboot_tag *next_tag(struct multiboot_tag *tag) {
    uint32_t sz = tag->size;
    uint8_t *p = (uint8_t *)tag + ((sz + 7U) & ~7U);
    return (struct multiboot_tag *)p;
}

int multiboot_validate(uint32_t magic, uint64_t mbi_phys) {
    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
        return 0;
    }
    if (mbi_phys & 7ULL) {
        return 0;
    }
    uint32_t total = *(uint32_t *)(uintptr_t)mbi_phys;
    if (total < 16 || total > 0x100000) {
        return 0;
    }
    return 1;
}

int multiboot_fill_vbe(uint64_t mbi_phys, struct vbe_mode_info *out) {
    struct multiboot_tag *tag =
        (struct multiboot_tag *)((uint8_t *)(uintptr_t)mbi_phys + 8);
    for (;;) {
        if (tag->type == MULTIBOOT_TAG_TYPE_END) {
            break;
        }
        if (tag->type == MULTIBOOT_TAG_TYPE_FRAMEBUFFER &&
            tag->size >= sizeof(struct multiboot_tag_framebuffer)) {
            struct multiboot_tag_framebuffer *fb =
                (struct multiboot_tag_framebuffer *)tag;
            out->width = (uint16_t)fb->framebuffer_width;
            out->height = (uint16_t)fb->framebuffer_height;
            out->pitch = (uint16_t)fb->framebuffer_pitch;
            out->bpp = fb->framebuffer_bpp;
            out->framebuffer = (uint32_t)fb->framebuffer_addr;
            return 0;
        }
        tag = next_tag(tag);
    }
    return -1;
}
