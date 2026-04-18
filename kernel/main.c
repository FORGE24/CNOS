/* kernel/main.c - 微内核主要入口点 */

#include <stdint.h>
#include "pmm.h"
#include "vmm.h"
#include "idt.h"
#include "shell.h"
#include "process.h"
#include "multiboot.h"
#include "fs/vfs.h"
#include "drivers/vga.h"
#include "drivers/serial.h"
#include "drivers/ide.h"
#include "fs/cnos/cnos_ext2_vol.h"

void putchar(char c) {
    vga_putchar(c);
    serial_putchar(c);
}

void puts(const char *s) {
    vga_puts(s);
    serial_puts(s);
}

void clear_screen(void) {
    vga_clear();
}

void puts_hex(uint64_t n) {
    char hex[] = "0123456789ABCDEF";
    puts("0x");
    for (int i = 60; i >= 0; i -= 4) {
        putchar(hex[(n >> i) & 0xF]);
    }
}

void puts_dec(uint64_t n) {
    if (n == 0) {
        putchar('0');
        return;
    }
    char buf[20];
    int i = 0;
    while (n > 0) {
        buf[i++] = (char)((n % 10) + '0');
        n /= 10;
    }
    while (--i >= 0) {
        putchar(buf[i]);
    }
}

static void test_task(void) {
    for (;;) {
        __asm__ volatile("hlt");
    }
}

void kernel_main(uint64_t mbi_phys, uint64_t boot_magic) {
    vga_init();
    serial_init();

    if (!multiboot_validate((uint32_t)boot_magic, mbi_phys)) {
        puts("CNOS: invalid Multiboot2 handoff\n");
        for (;;) {
            __asm__ volatile("hlt");
        }
    }

    pmm_init(mbi_phys);
    vmm_init();
    ide_init();
    cnos_vol_init();
    vfs_init();

    process_init();
    idt_init();
    shell_init();

    process_create(test_task);

    puts("Welcome to CNOS 64-bit Microkernel (VGA text + serial).\n");
    puts("Memory OK. VFS init done.\n\nMultiboot2 @ ");
    puts_hex(mbi_phys);
    puts("\nPhysical: ");
    puts_dec(pmm_get_total_memory() / 1024 / 1024);
    puts(" MB total, ");
    puts_dec(pmm_get_free_memory() / 1024 / 1024);
    puts(" MB free\n");

    __asm__ volatile("sti");
    puts("\nCNOS> ");

    for (;;) {
        __asm__ volatile("hlt");
    }
}
