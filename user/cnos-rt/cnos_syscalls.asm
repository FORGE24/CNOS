; CNOS 用户态系统调用桩 — 约定见 user/SYSCALL-ABI.txt、kernel/syscall_abi.h
; SysV amd64：参数通过 RDI、RSI、RDX（及预留 R10/R8/R9）传入；供 Slime extern "C" 链接。
;
; CNOS_SYS_WRITE = 1 : EAX=1, RDI=fd, RSI=buf, RDX=len → 返回 RAX
; CNOS_SYS_EXIT  = 0 : EAX=0, RDI=exit_code
; CNOS_SYS_GETPID = 2 : EAX=2 → RAX
; CNOS_SYS_READ   = 3 : EAX=3, RDI=fd, RSI=buf, RDX=len → RAX
; CNOS_SYS_UPTIME_TICKS = 4 : EAX=4 → RAX
; CNOS_SYS_OPEN = 5 : EAX=5, RDI=path, RSI=flags → RAX
; CNOS_SYS_CLOSE = 6 : EAX=6, RDI=fd → RAX

global cnos_sys_write
global cnos_sys_exit
global cnos_sys_getpid
global cnos_sys_read
global cnos_sys_uptime_ticks
global cnos_sys_open
global cnos_sys_close

section .text

cnos_sys_write:
    mov eax, 1
    int 0x80
    ret

cnos_sys_exit:
    xor eax, eax
    int 0x80
    ud2

cnos_sys_getpid:
    mov eax, 2
    int 0x80
    ret

cnos_sys_read:
    mov eax, 3
    int 0x80
    ret

cnos_sys_uptime_ticks:
    mov eax, 4
    int 0x80
    ret

cnos_sys_open:
    mov eax, 5
    int 0x80
    ret

cnos_sys_close:
    mov eax, 6
    int 0x80
    ret
