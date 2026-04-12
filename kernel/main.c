/* kernel/main.c - 微内核主要入口点 */

#include <stdint.h>
#include "pmm.h"
#include "vmm.h"
#include "idt.h"
#include "shell.h"
#include "process.h"
#include "drivers/vga.h"
#include "drivers/serial.h"
#include "drivers/graphics.h"
#include "drivers/gui.h"
#include "drivers/mouse.h"
#include "process.h"

/* E820 内存条目结构 */
struct e820_entry {
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t extended_attributes;
} __attribute__((packed));

/* 简单的输出函数封装 */
void putchar(char c) {
    vga_putchar(c);
    serial_putchar(c);
}

void puts(const char *s) {
    vga_puts(s);
    serial_puts(s);
}

void clear_screen() {
    vga_clear();
}

/* 打印十六进制数字 */
void puts_hex(uint64_t n) {
    char hex[] = "0123456789ABCDEF";
    puts("0x");
    for (int i = 60; i >= 0; i -= 4) {
        putchar(hex[(n >> i) & 0xF]);
    }
}

/* 打印十进制数字 */
void puts_dec(uint64_t n) {
    if (n == 0) {
        putchar('0');
        return;
    }
    char buf[20];
    int i = 0;
    while (n > 0) {
        buf[i++] = (n % 10) + '0';
        n /= 10;
    }
    while (--i >= 0) {
        putchar(buf[i]);
    }
}

void test_task() {
    while (1) {
        /* 在串口输出一些信息，证明任务在运行 */
        // serial_puts("Test Task Running...\n");
        __asm__ volatile("hlt");
    }
}

/* 内核主入口函数 */
void kernel_main(uint64_t mem_map_addr) {
    /* 1. 首先初始化最基础的硬件输出和内存管理 */
    vga_init();
    serial_init();
    pmm_init(mem_map_addr);
    vmm_init();
    
    /* 2. 初始化图形模式 (现在 PMM/VMM 已就绪，可以安全进行页表映射) */
    struct vbe_mode_info *vbe = (struct vbe_mode_info *)0x9200;
    graphics_init(vbe);

    /* 调试输出实际 VBE 信息 */
    puts("VBE Width: "); puts_dec(vbe->width); puts("\n");
    puts("VBE Height: "); puts_dec(vbe->height); puts("\n");
    puts("VBE Pitch: "); puts_dec(vbe->pitch); puts("\n");
    puts("VBE Framebuffer: "); puts_hex(vbe->framebuffer); puts("\n");

    /* 3. 初始化 GUI 界面 */
    gui_init();

    /* 4. 初始化其他核心系统 */
    process_init();
    idt_init();
    mouse_init();
    shell_init();

    /* 创建一个测试任务 */
    process_create(test_task);
    
    puts("Welcome to CNOS 64-bit Microkernel!\n");
    puts("VBE Mode Initialized (1024x768x32).\n");
    puts("Memory Management Initialized.\n");

    /* 解析内存地图 (仅用于输出调试信息) */
    uint16_t entry_count = *(uint16_t *)mem_map_addr;
    struct e820_entry *entries = (struct e820_entry *)(mem_map_addr + 4);

    puts("\nBIOS Memory Map (E820):\n");
    for (int i = 0; i < entry_count; i++) {
        puts_hex(entries[i].base);
        puts(" | ");
        puts_hex(entries[i].length);
        puts(" | ");
        puts_dec(entries[i].type);
        puts("\n");
    }
    
    puts("\nPhysical Memory:\n");
    puts("  Total: "); puts_dec(pmm_get_total_memory() / 1024 / 1024); puts(" MB\n");
    puts("  Free:  "); puts_dec(pmm_get_free_memory() / 1024 / 1024); puts(" MB\n");

    /* 开启全局中断并进入 Shell 准备状态 */
    __asm__ volatile("sti");
    puts("\nInteractive GUI Ready.\n");
    puts("CNOS> ");

    /* 进入内核主循环 */
    while (1) {
        __asm__ volatile("hlt");
    }
}
