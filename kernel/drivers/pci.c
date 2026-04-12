/* kernel/drivers/pci.c - PCI 总线驱动实现 */

#include "pci.h"
#include "../io.h"
#include <stdint.h>

extern void puts(const char *s);
extern void puts_hex(uint64_t n);
extern void putchar(char c);

uint32_t pci_read_config(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    uint32_t address = (uint32_t)((uint32_t)0x80000000 | ((uint32_t)bus << 16) | ((uint32_t)device << 11) | ((uint32_t)function << 8) | (offset & 0xFC));
    outb(PCI_CONFIG_ADDR, (uint8_t)(address & 0xFF));
    outb(PCI_CONFIG_ADDR + 1, (uint8_t)((address >> 8) & 0xFF));
    outb(PCI_CONFIG_ADDR + 2, (uint8_t)((address >> 16) & 0xFF));
    outb(PCI_CONFIG_ADDR + 3, (uint8_t)((address >> 24) & 0xFF));
    
    // 我们需要一个 32 位的 in 端口读取函数
    uint32_t ret;
    __asm__ volatile("inl %1, %0" : "=a"(ret) : "Nd"(PCI_CONFIG_DATA));
    return ret;
}

void pci_list_devices() {
    puts("PCI Devices List:\n");
    puts("Bus | Dev | Func | Vendor | Device\n");
    puts("----|-----|------|--------|--------\n");

    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t dev = 0; dev < 32; dev++) {
            for (uint8_t func = 0; func < 8; func++) {
                uint32_t data = pci_read_config(bus, dev, func, 0);
                uint16_t vendor_id = data & 0xFFFF;
                if (vendor_id != 0xFFFF) {
                    uint16_t device_id = (data >> 16) & 0xFFFF;
                    
                    puts_hex(bus); puts(" | ");
                    puts_hex(dev); puts(" | ");
                    puts_hex(func); puts(" | ");
                    puts_hex(vendor_id); puts(" | ");
                    puts_hex(device_id); puts("\n");
                }
            }
        }
    }
}
