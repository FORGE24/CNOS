/* hybrid_ipc.h — 混合内核 IPC 服务与用户态 open 走 IPC（阶段 2–3） */

#ifndef KERNEL_HYBRID_IPC_H
#define KERNEL_HYBRID_IPC_H

#include <stdint.h>

#define CHASEROS_HYBRID_SERVICE_PID 3ull

#define CHASEROS_MSG_NOP       0ull
#define CHASEROS_MSG_PING      1ull
#define CHASEROS_MSG_PONG      2ull
#define CHASEROS_MSG_FS_OPEN   10ull
#define CHASEROS_MSG_FS_REPLY  11ull

void chaseros_hybrid_ipc_service_spawn(void);

long hybrid_user_fd_open_via_ipc(const char *path, int flags);

#endif
