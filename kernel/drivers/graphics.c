/* kernel/drivers/graphics.c - 帧缓冲绘图 */

#include "graphics.h"
#include "font.h"
#include <stddef.h>
#include <stdint.h>

static struct vbe_mode_info *mode_info;
static uintptr_t lfb_base;

static int bpp_bytes(void) {
    if (!mode_info) {
        return 4;
    }
    int bpp = (int)mode_info->bpp;
    if (bpp <= 0) {
        return 4;
    }
    return (bpp + 7) / 8;
}

void graphics_init(struct vbe_mode_info *vbe) {
    mode_info = vbe;
    lfb_base = (uintptr_t)vbe->framebuffer;
}

/*
 * GRUB/QEMU 常见为 24 bpp（每像素 3 字节）或 32 bpp。此前固定按 x*4 写字节会导致与 pitch
 * 错位，出现整屏竖向 RGB 条纹与字体拉伸。
 */
void graphics_draw_pixel(int x, int y, uint32_t color) {
    if (!mode_info || lfb_base == 0) {
        return;
    }
    if (x < 0 || x >= mode_info->width || y < 0 || y >= mode_info->height) {
        return;
    }

    int bp = bpp_bytes();
    uint8_t *row = (uint8_t *)lfb_base + (size_t)y * (size_t)mode_info->pitch;
    uint8_t *p = row + (size_t)x * (size_t)bp;

    uint8_t r = (uint8_t)((color >> 16) & 0xFFu);
    uint8_t g = (uint8_t)((color >> 8) & 0xFFu);
    uint8_t b = (uint8_t)(color & 0xFFu);
    uint8_t a = (uint8_t)((color >> 24) & 0xFFu);

    int bpp = (int)mode_info->bpp;

    if (bpp == 32) {
        *(uint32_t *)p = color;
        return;
    }
    if (bpp >= 24) {
        /* 线性帧缓冲常用 BGR888 / 与 32bpp 低三字节一致 */
        p[0] = b;
        p[1] = g;
        p[2] = r;
        if (bp >= 4) {
            p[3] = a;
        }
        return;
    }
    if (bpp >= 16) {
        uint16_t px565 =
            (uint16_t)(((uint16_t)(r & 0xF8u) << 8) | ((uint16_t)(g & 0xFCu) << 3) | (uint16_t)(b >> 3));
        *(uint16_t *)p = px565;
        return;
    }
    /* 过低 bpp：尽力写灰度 */
    p[0] = (uint8_t)(((int)r + (int)g + (int)b) / 3);
}

void graphics_draw_rect(int x, int y, int w, int h, uint32_t color) {
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            graphics_draw_pixel(x + i, y + j, color);
        }
    }
}

void graphics_clear(uint32_t color) {
    if (!mode_info || lfb_base == 0) {
        return;
    }
    graphics_draw_rect(0, 0, mode_info->width, mode_info->height, color);
}

void graphics_scroll_up_pixels(int dy, uint32_t fill_color) {
    if (!mode_info || lfb_base == 0 || dy <= 0) {
        return;
    }
    int h = mode_info->height;
    int pitch = mode_info->pitch;
    int w = mode_info->width;
    if (dy >= h) {
        graphics_clear(fill_color);
        return;
    }
    uint8_t *base = (uint8_t *)lfb_base;
    for (int row = 0; row < h - dy; row++) {
        uint8_t *dst = base + (size_t)row * (size_t)pitch;
        uint8_t *src = base + (size_t)(row + dy) * (size_t)pitch;
        for (int i = 0; i < pitch; i++) {
            dst[i] = src[i];
        }
    }
    graphics_draw_rect(0, h - dy, w, dy, fill_color);
}

void graphics_draw_char(int x, int y, char c, uint32_t color) {
    graphics_draw_char_scaled(x, y, c, color, 1);
}

void graphics_draw_char_scaled(int x, int y, char c, uint32_t color, int scale) {
    if (!mode_info || lfb_base == 0 || scale < 1) {
        return;
    }
    if ((unsigned char)c > 127) {
        return;
    }

    unsigned char *bitmap = font8x16[(unsigned char)c];
    for (int j = 0; j < 16; j++) {
        for (int i = 0; i < 8; i++) {
            if (bitmap[j] & (1 << (7 - i))) {
                for (int sy = 0; sy < scale; sy++) {
                    for (int sx = 0; sx < scale; sx++) {
                        graphics_draw_pixel(x + i * scale + sx, y + j * scale + sy, color);
                    }
                }
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

int graphics_get_width(void) {
    return mode_info ? mode_info->width : 0;
}

int graphics_get_height(void) {
    return mode_info ? mode_info->height : 0;
}

int graphics_get_pitch(void) {
    return mode_info ? mode_info->pitch : 0;
}

uint32_t *graphics_get_lfb(void) {
    return (uint32_t *)(void *)lfb_base;
}
