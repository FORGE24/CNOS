/* 最小用户 ELF 模板 — 逻辑需与 chaseros_user.h / SYSCALL-ABI.txt 一致 */

#include "chaseros_user.h"

void _start(void)
{
    static const char msg[] = "ChaserOS user minimal\n";
    (void)chaseros_syscall_write(1, msg, sizeof(msg) - 1);
    chaseros_syscall_exit(0);
}
