/* kernel/drivers/graphics.h - 基础图形驱动 */

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>

/* VBE 模式信息结构 (从引导程序传递) */
struct vbe_mode_info {
    uint16_t attributes;
    uint8_t window_a, window_b;
    uint16_t granularity;
    uint16_t window_size;
    uint16_t segment_a, segment_b;
    uint32_t win_func_ptr;
    uint16_t pitch;             /* 每行字节数 */
    uint16_t width, height;     /* 屏幕宽高 */
    uint8_t w_char, y_char, planes, bpp, banks;
    uint8_t memory_model, bank_size, image_pages;
    uint8_t reserved0;
    
    uint8_t red_mask, red_position;
    uint8_t green_mask, green_position;
    uint8_t blue_mask, blue_position;
    uint8_t reserved_mask, reserved_position;
    uint8_t direct_color_attributes;
    
    uint32_t framebuffer;       /* 线性帧缓冲区物理地址 (LFB) */
    uint32_t off_screen_mem_off;
    uint16_t off_screen_mem_size;
    uint8_t reserved1[206];
} __attribute__((packed));

void graphics_init(struct vbe_mode_info *vbe);
void graphics_draw_pixel(int x, int y, uint32_t color);
void graphics_draw_rect(int x, int y, int w, int h, uint32_t color);
void graphics_clear(uint32_t color);
void graphics_draw_char(int x, int y, char c, uint32_t color);
void graphics_draw_string(int x, int y, const char *s, uint32_t color);
int graphics_get_width();
int graphics_get_height();
int graphics_get_pitch();
uint32_t *graphics_get_lfb();

#endif
