/* kernel/isr.c - 中断处理 C 程序 */

#include <stdint.h>
#include <stddef.h>
#include "io.h"
#include "shell.h"
#include "drivers/mouse.h"
#include "process.h"

/* 寄存器结构 */
struct registers {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rdi, rsi, rbp, rdx, rcx, rbx, rax;
    uint64_t int_no, err_code;
    uint64_t rip, cs, rflags, rsp, ss;
};

/* 键盘扫描码表 (简化版) */
static const char scancode_ascii[] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
};

/* 外部函数声明 */
extern void puts(const char *s);
extern void puts_hex(uint64_t n);
extern void putchar(char c);

/* 中断处理主程序 */
void isr_handler(struct registers *regs) {
    if (regs->int_no < 32) {
        /* 处理异常 */
        puts("\n!!! CPU EXCEPTION !!!\n");
        puts("Exception Number: ");
        char hex[] = "0123456789ABCDEF";
        putchar(hex[regs->int_no / 16]);
        putchar(hex[regs->int_no % 16]);
        puts("\nError Code: ");
        puts_hex(regs->err_code);
        puts("\nRIP: ");
        puts_hex(regs->rip);
        puts("\nCS: ");
        puts_hex(regs->cs);
        puts("\nStopping system...\n");
        
        while (1) {
            __asm__ volatile("hlt");
        }
    } else if (regs->int_no >= 32 && regs->int_no <= 47) {
        /* 处理 IRQ */
        if (regs->int_no == 32) { /* IRQ 0: 时钟 */
            schedule();
        } else if (regs->int_no == 33) { /* IRQ 1: 键盘 */
            uint8_t scancode = inb(0x60);
            /* 只处理按下 (按下 < 0x80) */
            if (scancode < 0x80) {
                if (scancode < sizeof(scancode_ascii)) {
                    char c = scancode_ascii[scancode];
                    if (c) {
                        shell_handle_input(c);
                    }
                }
            }
        } else if (regs->int_no == 44) { /* IRQ 12: 鼠标 */
            mouse_handler();
        }

        /* 发送 EOI (End of Interrupt) */
        if (regs->int_no >= 40) {
            outb(0xA0, 0x20); /* 发送到从片 */
        }
        outb(0x20, 0x20);     /* 发送到主片 */
    } else if (regs->int_no == 128) {
        /* 处理系统调用 */
        puts("\nSystem Call Triggered (0x80)!\n");
    }
}
