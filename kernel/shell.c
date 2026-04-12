/* kernel/shell.c - 基础 Shell 实现 */

#include "shell.h"
#include "pmm.h"
#include "io.h"
#include "drivers/vga.h"
#include "drivers/pci.h"
#include <stdint.h>
#include <stddef.h>

extern void puts(const char *s);
extern void putchar(char c);
extern void vga_clear();
extern void puts_dec(uint64_t n);

static char cmd_buffer[MAX_COMMAND_LEN];
static int cmd_len = 0;

/* ... strcmp 保持不变 ... */
static int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

void shell_init() {
    for (int i = 0; i < MAX_COMMAND_LEN; i++) cmd_buffer[i] = 0;
    cmd_len = 0;
}

void shell_execute(const char *cmd) {
    if (strcmp(cmd, "help") == 0) {
        puts("Available commands:\n");
        puts("  help   - Show this help message\n");
        puts("  clear  - Clear the screen\n");
        puts("  mem    - Show memory statistics\n");
        puts("  reboot - Reboot the system\n");
        puts("  ls     - List files (simulation)\n");
        puts("  ps     - List processes (simulation)\n");
        puts("  lspci  - List PCI devices\n");
    } else if (strcmp(cmd, "clear") == 0) {
        vga_clear();
    } else if (strcmp(cmd, "mem") == 0) {
        puts("Memory Statistics:\n");
        puts("  Total: "); puts_dec(pmm_get_total_memory() / 1024 / 1024); puts(" MB\n");
        puts("  Used:  "); puts_dec(pmm_get_used_memory() / 1024); puts(" KB\n");
        puts("  Free:  "); puts_dec(pmm_get_free_memory() / 1024 / 1024); puts(" MB\n");
    } else if (strcmp(cmd, "reboot") == 0) {
        puts("Rebooting...\n");
        uint8_t good = 0x02;
        while (good & 0x02)
            good = inb(0x64);
        outb(0x64, 0xFE);
    } else if (strcmp(cmd, "ls") == 0) {
        puts("Simulation File System:\n");
        puts("  /bin/shell\n");
        puts("  /etc/os-release\n");
        puts("  /dev/vga\n");
        puts("  /dev/serial\n");
    } else if (strcmp(cmd, "ps") == 0) {
        puts("Simulation Process List:\n");
        puts(" PID | State | Name\n");
        puts("-----|-------|------\n");
        puts(" 0   | R     | Kernel\n");
        puts(" 1   | R     | Shell\n");
    } else if (strcmp(cmd, "lspci") == 0) {
        pci_list_devices();
    } else if (cmd[0] != '\0') {
        puts("Unknown command: ");
        puts(cmd);
        puts("\n");
    }
}

void shell_handle_input(char c) {
    if (c == '\n') {
        putchar('\n');
        cmd_buffer[cmd_len] = '\0';
        shell_execute(cmd_buffer);
        cmd_len = 0;
        puts("CNOS> ");
    } else if (c == '\b') {
        if (cmd_len > 0) {
            cmd_len--;
            putchar('\b');
        }
    } else {
        if (cmd_len < MAX_COMMAND_LEN - 1) {
            cmd_buffer[cmd_len++] = c;
            putchar(c);
        }
    }
}
