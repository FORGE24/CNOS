/* syscall_abi.h — CNOS 用户态系统调用编号与寄存器角色（单一事实来源）
 *
 * 陷阱：int $0x80（CPU 向量 0x80 → IDT 项 128），仅当 CS.CPL=3 时由 isr.c 处理。
 * 用户参数采用与 System V AMD64 一致的寄存器顺序，便于 C / Slime extern "C" 直接调用。
 *
 * 完整说明见仓库 user/SYSCALL-ABI.txt
 */

#ifndef KERNEL_SYSCALL_ABI_H
#define KERNEL_SYSCALL_ABI_H

/** IDT 中软件中断向量（与 interrupts.asm 中门一致） */
#define CNOS_SYSCALL_VECTOR 0x80u

/* -------------------------------------------------------------------------- */
/* 系统调用号（RAX 低 64 位；桩代码常用 EAX，须与此一致）                     */
/* -------------------------------------------------------------------------- */

#define CNOS_SYS_EXIT         0u
#define CNOS_SYS_WRITE        1u
#define CNOS_SYS_GETPID       2u
#define CNOS_SYS_READ         3u
#define CNOS_SYS_UPTIME_TICKS 4u
#define CNOS_SYS_OPEN          5u
#define CNOS_SYS_CLOSE         6u

/** read/write 等单次传输上限（字节），防止恶意极大 len */
#define CNOS_SYSCALL_MAX_IO_BYTES (1024u * 1024u)
#define CNOS_SYSCALL_MAX_WRITE_BYTES CNOS_SYSCALL_MAX_IO_BYTES

/* 预留分配策略：0–255 由内核与 user/SYSCALL-ABI.txt 固定；扩展需同步
 * integrations/slime-for-cnos/std/cnos/constants.sm 与 user/cnos-rt/cnos_syscalls.asm */

/* -------------------------------------------------------------------------- */
/* 寄存器角色（入口时；由用户态保证，内核从 struct registers 读取）          */
/* -------------------------------------------------------------------------- */

/* RAX = CNOS_SYS_* */

/* 参数 1–6：与 SysV amd64 整数参数顺序一致（第 4–6 个预留未来 syscall） */
/* RDI = arg1, RSI = arg2, RDX = arg3, R10 = arg4, R8 = arg5, R9 = arg6 */

/* 返回值置于 RAX：成功为非负；失败为负 errno（见 kernel/errno.h、user/SYSCALL-ABI.txt） */

/* CNOS_SYS_EXIT: RDI = 退出码（内核可选用） */

/* CNOS_SYS_WRITE: RDI = fd, RSI = 缓冲区用户 VA, RDX = 长度；RAX = 已写字节数或负 errno */

/* CNOS_SYS_GETPID: 无参；RAX = 当前用户任务 PID */

/* CNOS_SYS_READ: RDI = fd, RSI = 缓冲区用户 VA, RDX = 长度；RAX = 已读字节数或负 errno */

/* CNOS_SYS_UPTIME_TICKS: 无参；RAX = 内核单调 tick（自启动以来 IRQ0 次数） */

/* CNOS_SYS_OPEN: RDI = 路径用户 VA（根目录文件名）；RSI = flags（预留，O_RDONLY=0）；RAX = fd 或 -errno */

/* CNOS_SYS_CLOSE: RDI = fd */

#endif /* KERNEL_SYSCALL_ABI_H */
