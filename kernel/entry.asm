; kernel/entry.asm - 32位到64位长模式的转换
; TAB=4

SECTION .text
[BITS 32]
extern kernel_main
extern _bss_start
extern _bss_end
global _start

_start:
    ; 保存从引导程序传递过来的内存地图地址 (ESI)
    mov ebx, esi

    ; --- 显式清零整个 BSS 段 ---
    mov edi, _bss_start
    mov ecx, _bss_end
    sub ecx, edi
    xor eax, eax
    rep stosb

    ; --- 开启 SSE (C 语言代码可能需要) ---
    mov eax, cr0
    and ax, 0xFFFB      ; 清除 CR0.EM
    or ax, 0x0002       ; 设置 CR0.MP
    mov cr0, eax
    mov eax, cr4
    or ax, 3 << 9       ; 设置 CR4.OSFXSR 和 CR4.OSXMMEXCPT
    mov cr4, eax

    ; --- 设置64位分页 ---
    ; PML4: 指向 PDPT
    mov eax, pdpt_table
    or eax, 0x03        ; 存在, 可读写
    mov [pml4_table], eax

    ; PDPT: 指向 PD
    mov eax, pd_table
    or eax, 0x03        ; 存在, 可读写
    mov [pdpt_table], eax

    ; --- 映射 4GB 内存 (PML4[0] -> PDPT[0..3] -> PD) ---
    ; 我们需要 4 个 PD 表来映射 4GB (每个 PD 映射 1GB)
    
    ; PDPT 条目 0, 1, 2, 3 分别指向 4 个 PD 表
    mov eax, pd_table
    or eax, 0x03
    mov [pdpt_table], eax
    add eax, 4096
    mov [pdpt_table + 8], eax
    add eax, 4096
    mov [pdpt_table + 16], eax
    add eax, 4096
    mov [pdpt_table + 24], eax

    ; 填充 4 个 PD 表 (共 2048 个 2MB 页面)
    mov edi, pd_table
    mov ecx, 2048       ; 512 * 4
    xor ebx, ebx        ; 物理地址起始为 0
.map_4gb:
    mov eax, ebx
    or eax, 0x83        ; 存在, 可读写, 巨大页
    mov [edi], eax
    mov dword [edi + 4], 0
    add edi, 8
    add ebx, 0x200000   ; 每个条目增加 2MB
    loop .map_4gb

    ; 加载 CR3 寄存器 (PML4 表地址)
    mov eax, pml4_table
    mov cr3, eax

    ; 开启 PAE (物理地址扩展)
    mov eax, cr4
    or eax, 0x20
    mov cr4, eax

    ; 切换到长模式 (设置 EFER.LME)
    mov ecx, 0xC0000080
    rdmsr
    or eax, 0x100
    wrmsr

    ; 开启分页
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    ; 跳转到 64位长模式
    lgdt [gdt64_ptr]
    jmp 0x08:long_mode_entry

[BITS 64]
long_mode_entry:
    ; 重置段寄存器
    mov ax, 0
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax

    ; 设置内核栈
    mov rsp, stack_top

    ; 将内存地图地址传递给 kernel_main (第一个参数在 RDI)
    mov rdi, rbx

    ; 调用 C 语言编写的内核主函数
    call kernel_main

    ; 如果内核返回，进入停机状态
.halt:
    hlt
    jmp .halt

; 64位 GDT
SECTION .data
align 16
gdt64_start:
    dq 0x0
    dq 0x0020980000000000 ; 代码段 (64位)
    dq 0x0000920000000000 ; 数据段
gdt64_end:

gdt64_ptr:
    dw gdt64_end - gdt64_start - 1
    dd gdt64_start      ; 在32位模式下使用 dd (4字节) 存储基地址

; 页表
SECTION .bss
align 4096
pml4_table:
    resb 4096
pdpt_table:
    resb 4096
pd_table:
    resb 4096 * 4       ; 预留 4 个 PD 表以支持 4GB 映射

; 内核栈
align 16
stack_bottom:
    resb 4096
stack_top:
