/* kernel/main.c - 微内核主要入口点 */

#include <stdint.h>
#include "pmm.h"
#include "vmm.h"
#include "idt.h"
#include "shell.h"
#include "process.h"
#include "multiboot.h"
#include "drivers/vga.h"
#include "drivers/serial.h"
#include "drivers/graphics.h"
#include "drivers/gui.h"
#include "drivers/mouse.h"

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
        __asm__ volatile("hlt");
    }
}

/* 内核主入口函数 (rdi=Multiboot2 信息物理地址, rsi=引导魔数) */
void kernel_main(uint64_t mbi_phys, uint64_t boot_magic) {
    vga_init();
    serial_init();

    if (!multiboot_validate((uint32_t)boot_magic, mbi_phys)) {
        serial_puts("CNOS: invalid Multiboot2 handoff\n");
        while (1) {
            __asm__ volatile("hlt");
        }
    }

    pmm_init(mbi_phys);
    vmm_init();

    struct vbe_mode_info vbe;
    for (unsigned i = 0; i < sizeof(vbe); i++) {
        ((uint8_t *)&vbe)[i] = 0;
    }
    if (multiboot_fill_vbe(mbi_phys, &vbe) != 0) {
        puts("Warning: no Multiboot framebuffer tag\n");
    }
    graphics_init(&vbe);

    puts("Framebuffer Width: ");
    puts_dec(vbe.width);
    puts("\n");
    puts("Framebuffer Height: ");
    puts_dec(vbe.height);
    puts("\n");
    puts("Framebuffer Pitch: ");
    puts_dec(vbe.pitch);
    puts("\n");
    puts("Framebuffer Addr: ");
    puts_hex(vbe.framebuffer);
    puts("\n");

    gui_init();

    process_init();
    idt_init();
    mouse_init();
    shell_init();

    process_create(test_task);

    puts("Welcome to CNOS 64-bit Microkernel!\n");
    puts("Boot via GRUB2 (Multiboot2).\n");
    puts("Memory Management Initialized.\n");

    puts("\nMultiboot2 info @ ");
    puts_hex(mbi_phys);
    puts("\n");

    puts("\nPhysical Memory:\n");
    puts("  Total: ");
    puts_dec(pmm_get_total_memory() / 1024 / 1024);
    puts(" MB\n");
    puts("  Free:  ");
    puts_dec(pmm_get_free_memory() / 1024 / 1024);
    puts(" MB\n");

    __asm__ volatile("sti");
    puts("\nInteractive GUI Ready.\n");
    puts("CNOS> ");

    while (1) {
        __asm__ volatile("hlt");
    }
}
