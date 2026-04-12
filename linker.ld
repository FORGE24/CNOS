/* linker.ld - CNOS 64位微内核链接脚本 */

ENTRY(_start)

SECTIONS
{
    /* 
     * 内核加载地址设为 0x8000 以匹配 boot.asm。
     * 0x8000 位于引导程序之上，且不会覆盖引导程序。
     */
    . = 0x8000;

    .text :
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
