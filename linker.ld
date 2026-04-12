/* linker.ld - CNOS 64位微内核链接脚本 (Multiboot2 / GRUB2) */

ENTRY(_start)

SECTIONS
{
    /*
     * 1MiB 处加载，符合 Multiboot2 / GRUB 常用布局。
     */
    . = 0x100000;

    _kernel_start = .;

    .multiboot ALIGN(8) :
    {
        *(.multiboot)
    }

    .text ALIGN(8) :
    {
        *(.text)
    }

    .rodata :
    {
        *(.rodata)
    }

    .data :
    {
        *(.data)
    }

    .bss :
    {
        _bss_start = .;
        *(.bss)
        *(COMMON)
        _bss_end = .;
    }

    _kernel_end = .;

    /DISCARD/ :
    {
        *(.comment)
        *(.eh_frame)
    }
}
