ChaserOS Slime 用户运行时（汇编桩）
==============================

**chaseros_syscalls.asm** 导出符号：

  * **chaseros_sys_write** — **CHASEROS_SYS_WRITE**，参数 **RDI/RDX/RSI**（SysV），返回 **RAX**
  * **chaseros_sys_exit** — **CHASEROS_SYS_EXIT**，**RDI** = 退出码

供 Slime **`extern "C"`** 与 **integrations/slime-for-chaseros/std/chaseros/io.sm** 链接。

纯 **C** 程序可直接使用 **user/include/chaseros_user.h** 内联 **`int $0x80`**，不必链接本文件。

链接顺序（**scripts/cnos-slime-compile.sh**）：

  nasm -felf64 app.asm -o app.o
  nasm -felf64 chaseros_syscalls.asm -o chaseros_rt.o
  x86_64-elf-ld -nostdlib -T user/user.ld -o app.elf app.o chaseros_rt.o
