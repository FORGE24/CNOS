/* kernel/process.h - 进程/线程控制块 (PCB/TCB) */

#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include <stddef.h>
#include "ipc.h"

/* 进程状态定义 */
typedef enum {
    PROC_STATE_READY,     /* 就绪态 */
    PROC_STATE_RUNNING,   /* 运行态 */
    PROC_STATE_SENDING,   /* 阻塞：等待发送消息 */
    PROC_STATE_RECEIVING, /* 阻塞：等待接收消息 */
    PROC_STATE_SLEEPING,  /* 阻塞：休眠 */
    PROC_STATE_EXITED     /* 退出 */
} proc_state_t;

/* 进程控制块 (PCB) 结构 */
typedef struct proc {
    uint64_t pid;             /* 进程 ID */
    proc_state_t state;       /* 进程状态 */
    uint64_t *stack_top;      /* 内核栈顶指针 (用于上下文切换) */
    uint64_t pml4_phys;       /* 虚拟内存地址空间 (CR3) */
    
    /* IPC 相关字段 */
    message_t msg_buffer;     /* 消息缓冲区 */
    struct proc *next_in_queue; /* 用于等待队列的链表指针 */
    struct proc *sender_queue;  /* 等待向此进程发送消息的进程队列 */
} proc_t;

/* 获取当前正在运行的进程 */
proc_t *get_current_process();

/* 初始化进程管理系统 */
void process_init();

#endif
