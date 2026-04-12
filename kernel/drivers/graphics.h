/* kernel/drivers/graphics.h - 帧缓冲绘图（未编入内核时仅保留头；与 multiboot 共用 vbe_mode_info） */

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>
#include "../multiboot.h"

void graphics_init(struct vbe_mode_info *vbe);
void graphics_draw_pixel(int x, int y, uint32_t color);
void graphics_draw_rect(int x, int y, int w, int h, uint32_t color);
void graphics_clear(uint32_t color);
void graphics_draw_char(int x, int y, char c, uint32_t color);
void graphics_draw_string(int x, int y, const char *s, uint32_t color);
int graphics_get_width(void);
int graphics_get_height(void);
int graphics_get_pitch(void);
uint32_t *graphics_get_lfb(void);

#endif
