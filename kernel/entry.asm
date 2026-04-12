; kernel/entry.asm - Multiboot2 入口，32位到64位长模式
; TAB=4

SECTION .multiboot
ALIGN 8
header_start:
    dd 0xE85250D6
    dd 0
    dd header_end - header_start
    dd 0x17ADA7BD
    ; framebuffer 请求: 1024x768x32
    dw 5, 0
    dd 20
    dd 1024
    dd 768
    dd 32
    dw 0, 0
    dd 8
header_end:

SECTION .text
[BITS 32]
extern kernel_main
extern _bss_start
extern _bss_end
global _start

SECTION .data
align 4
saved_magic:
    dd 0
saved_mbi:
    dd 0

SECTION .text
_start:
    mov [saved_magic], eax
    mov [saved_mbi], ebx

    mov esp, stack32_top

    ; --- 显式清零整个 BSS 段 ---
    mov edi, _bss_start
    mov ecx, _bss_end
    sub ecx, edi
    xor eax, eax
    rep stosb

    ; --- 开启 SSE (C 语言代码可能需要) ---
    mov eax, cr0
    and ax, 0xFFFB
    or ax, 0x0002
    mov cr0, eax
    mov eax, cr4
    or ax, 3 << 9
    mov cr4, eax

    ; --- 设置64位分页 ---
    mov eax, pdpt_table
    or eax, 0x03
    mov [pml4_table], eax

    mov eax, pd_table
    or eax, 0x03
    mov [pdpt_table], eax

    mov eax, pd_table
    or eax, 0x03
    mov [pdpt_table], eax
    add eax, 4096
    mov [pdpt_table + 8], eax
    add eax, 4096
    mov [pdpt_table + 16], eax
    add eax, 4096
    mov [pdpt_table + 24], eax

    mov edi, pd_table
    mov ecx, 2048
    xor ebx, ebx
.map_4gb:
    mov eax, ebx
    or eax, 0x83
    mov [edi], eax
    mov dword [edi + 4], 0
    add edi, 8
    add ebx, 0x200000
    loop .map_4gb

    mov eax, pml4_table
    mov cr3, eax

    mov eax, cr4
    or eax, 0x20
    mov cr4, eax

    mov ecx, 0xC0000080
    rdmsr
    or eax, 0x100
    wrmsr

    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    lgdt [gdt64_ptr]
    jmp 0x08:long_mode_entry

[BITS 64]
long_mode_entry:
    mov ax, 0
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax

    mov rsp, stack_top

    mov eax, [saved_mbi]
    mov rdi, rax

    mov eax, [saved_magic]
    mov rsi, rax

    call kernel_main

.halt:
    hlt
    jmp .halt

SECTION .data
align 16
gdt64_start:
    dq 0x0
    dq 0x0020980000000000
    dq 0x0000920000000000
gdt64_end:

gdt64_ptr:
    dw gdt64_end - gdt64_start - 1
    dd gdt64_start

SECTION .bss
align 16
stack32_bottom:
    resb 4096
stack32_top:

align 4096
pml4_table:
    resb 4096
pdpt_table:
    resb 4096
pd_table:
    resb 4096 * 4

align 16
stack_bottom:
    resb 4096
stack_top:
