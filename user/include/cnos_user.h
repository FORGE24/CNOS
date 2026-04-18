/* cnos_user.h — CNOS 裸机用户 ELF 公共 ABI（freestanding）
 *
 * 约定与 kernel/syscall_abi.h、user/SYSCALL-ABI.txt、integrations/slime-for-cnos/std 一致。
 * 链接脚本：user/user.ld；入口符号：_start
 */

#ifndef CNOS_USER_H
#define CNOS_USER_H

#include <stddef.h>

/* ---- 调用号（须与内核 CNOS_SYS_* 一致） -------------------------------- */
#define CNOS_SYS_EXIT          0
#define CNOS_SYS_WRITE         1
#define CNOS_SYS_GETPID        2
#define CNOS_SYS_READ          3
#define CNOS_SYS_UPTIME_TICKS  4
#define CNOS_SYS_OPEN          5
#define CNOS_SYS_CLOSE         6

#define CNOS_MAX_IO_LEN (1024u * 1024u)
#define CNOS_MAX_WRITE_LEN CNOS_MAX_IO_LEN

/* ---- errno（正数；若系统调用返回负值，则 errno = -返回值） ------------ */
#define CNOS_EPERM      1
#define CNOS_ENOENT     2
#define CNOS_EIO        5
#define CNOS_EBADF      9
#define CNOS_EMFILE     24
#define CNOS_EINVAL     22
#define CNOS_EFAULT     14
#define CNOS_ENOSYS     38

/**
 * SYS_WRITE：fd 1=stdout、2=stderr（均镜像控制台）；返回值在 RAX（≥0 为字节数；<0 为 -errno）。
 */
static inline long cnos_syscall_write(int fd, const void *buf, size_t len)
{
    long ret;
    __asm__ volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"((long)CNOS_SYS_WRITE),
          "D"((long)fd),
          "S"(buf),
          "d"(len)
        : "memory", "rcx", "r11");
    return ret;
}

/**
 * SYS_READ：当前 fd==0 (stdin) 无输入设备时返回 0（EOF）；缓冲区须可写映射。
 */
static inline long cnos_syscall_read(int fd, void *buf, size_t len)
{
    long ret;
    __asm__ volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"((long)CNOS_SYS_READ),
          "D"((long)fd),
          "S"(buf),
          "d"(len)
        : "memory", "rcx", "r11");
    return ret;
}

static inline long cnos_syscall_getpid(void)
{
    long ret;
    __asm__ volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"((long)CNOS_SYS_GETPID)
        : "memory", "rcx", "r11");
    return ret;
}

static inline long cnos_syscall_uptime_ticks(void)
{
    long ret;
    __asm__ volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"((long)CNOS_SYS_UPTIME_TICKS)
        : "memory", "rcx", "r11");
    return ret;
}

/** SYS_OPEN：path 为用户 VA；flags 预留（0=只读）；返回 fd≥3 或负 errno */
static inline long cnos_syscall_open(const char *path, int flags)
{
    long ret;
    __asm__ volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"((long)CNOS_SYS_OPEN),
          "D"(path),
          "S"((long)flags)
        : "memory", "rcx", "r11");
    return ret;
}

static inline long cnos_syscall_close(int fd)
{
    long ret;
    __asm__ volatile(
        "int $0x80"
        : "=a"(ret)
        : "a"((long)CNOS_SYS_CLOSE),
          "D"((long)fd)
        : "memory", "rcx", "r11");
    return ret;
}

/**
 * SYS_EXIT：不返回用户态；code 置于 RDI（内核可选用）。
 */
static inline void cnos_syscall_exit(int code)
{
    __asm__ volatile(
        "int $0x80"
        :
        : "a"((long)CNOS_SYS_EXIT), "D"((long)code)
        : "memory");
    __builtin_unreachable();
}

#endif /* CNOS_USER_H */
