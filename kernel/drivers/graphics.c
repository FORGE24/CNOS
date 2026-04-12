/* kernel/drivers/graphics.c - 基础图形驱动实现 (更新版) */

#include "graphics.h"
#include "font.h"
#include "../vmm.h"
#include <stddef.h>

static struct vbe_mode_info *mode_info;
static uint32_t *lfb;

void graphics_init(struct vbe_mode_info *vbe) {
    mode_info = vbe;
    
    /* 
     * 获取线性帧缓冲区物理地址 (LFB)。
     */
    lfb = (uint32_t *)(uint64_t)vbe->framebuffer;

    /* 
     * 如果显存地址为空，说明 VBE 初始化彻底失败
     */
    if (lfb == NULL) {
        return;
    }
}

void graphics_draw_pixel(int x, int y, uint32_t color) {
    if (x < 0 || x >= mode_info->width || y < 0 || y >= mode_info->height) {
        return;
    }
    
    /* 
     * 关键修复：
     * 根据 VBE 规范，显存地址 = LFB + (y * Pitch) + (x * BytesPerPixel)
     * 在 32位模式下 BytesPerPixel = 4
     */
    uint8_t *pixel_ptr = (uint8_t *)lfb + (y * (uint64_t)mode_info->pitch) + (x * 4);
    *(uint32_t *)pixel_ptr = color;
}

void graphics_draw_rect(int x, int y, int w, int h, uint32_t color) {
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            graphics_draw_pixel(x + i, y + j, color);
        }
    }
}

void graphics_clear(uint32_t color) {
    graphics_draw_rect(0, 0, mode_info->width, mode_info->height, color);
}

void graphics_draw_char(int x, int y, char c, uint32_t color) {
    if (c > 127) return;
    
    unsigned char *bitmap = font8x16[(unsigned char)c];
    for (int j = 0; j < 16; j++) {
        for (int i = 0; i < 8; i++) {
            if (bitmap[j] & (1 << (7 - i))) {
                graphics_draw_pixel(x + i, y + j, color);
            }
        }
    }
}

void graphics_draw_string(int x, int y, const char *s, uint32_t color) {
    int cur_x = x;
    while (*s) {
        if (*s == '\n') {
            cur_x = x;
            y += 16;
        } else {
            graphics_draw_char(cur_x, y, *s, color);
            cur_x += 8;
        }
        s++;
    }
}

int graphics_get_width() {
    return mode_info->width;
}

int graphics_get_height() {
    return mode_info->height;
}

int graphics_get_pitch() {
    return mode_info->pitch;
}

uint32_t *graphics_get_lfb() {
    return lfb;
}
