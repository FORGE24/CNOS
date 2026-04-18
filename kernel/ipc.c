/* kernel/ipc.c - 微内核进程间通信 (IPC) 实现 */

#include "ipc.h"
#include "process.h"
#include <stdint.h>
#include <stddef.h>

/*
 * 同步 IPC：Send / Receive。尚未接真实阻塞与抢占时：
 * - Receive 可先返回 IPC_WOULD_BLOCK 并将当前进程标为 RECEIVING；
 * - 随后 Send 可向 RECEIVING 目标投递并置 msg_pending，或把发送者挂入 sender_queue。
 */

static void msg_copy_to_buffer(proc_t *dest, proc_t *src, const message_t *msg) {
    dest->msg_buffer.sender = src->pid;
    dest->msg_buffer.type = msg->type;
    uint8_t *s = (uint8_t *)msg->payload;
    uint8_t *d = (uint8_t *)dest->msg_buffer.payload;
    for (int i = 0; i < MAX_MSG_PAYLOAD; i++) {
        d[i] = s[i];
    }
}

static void msg_copy_out(message_t *out, const proc_t *from) {
    out->sender = from->msg_buffer.sender;
    out->type = from->msg_buffer.type;
    uint8_t *s = (uint8_t *)from->msg_buffer.payload;
    uint8_t *d = (uint8_t *)out->payload;
    for (int i = 0; i < MAX_MSG_PAYLOAD; i++) {
        d[i] = s[i];
    }
}

static void stash_outgoing(proc_t *sender, const message_t *msg) {
    msg_copy_to_buffer(sender, sender, msg);
}

static void sender_enqueue(proc_t *dest, proc_t *sender) {
    sender->next_in_queue = dest->sender_queue;
    dest->sender_queue = sender;
}

/* src_pid==0 表示任意发送者；否则取队列中首个匹配的 PID */
static proc_t *sender_dequeue(proc_t *recv, uint64_t src_pid) {
    proc_t **pp = &recv->sender_queue;
    while (*pp) {
        proc_t *s = *pp;
        if (src_pid == 0 || s->pid == src_pid) {
            *pp = s->next_in_queue;
            s->next_in_queue = NULL;
            return s;
        }
        pp = &s->next_in_queue;
    }
    return NULL;
}

int ipc_send(uint64_t dest_pid, message_t *msg) {
    if (!msg) {
        return IPC_ERROR;
    }

    proc_t *current = get_current_process();
    proc_t *dest = process_find_by_pid(dest_pid);

    if (!dest || dest->state == PROC_STATE_FREE) {
        return IPC_ERROR;
    }
    if (dest == current) {
        return IPC_ERROR;
    }

    if (dest->state == PROC_STATE_RECEIVING) {
        msg_copy_to_buffer(dest, current, msg);
        dest->msg_pending = 1;
        dest->state = PROC_STATE_READY;
        return IPC_OK;
    }

    stash_outgoing(current, msg);
    current->state = PROC_STATE_SENDING;
    sender_enqueue(dest, current);
    return IPC_OK;
}

int ipc_receive(uint64_t src_pid, message_t *msg) {
    if (!msg) {
        return IPC_ERROR;
    }

    proc_t *current = get_current_process();

    if (current->msg_pending &&
        (src_pid == 0 || current->msg_buffer.sender == src_pid)) {
        msg_copy_out(msg, current);
        current->msg_pending = 0;
        if (current->state == PROC_STATE_RECEIVING) {
            current->state = PROC_STATE_READY;
        }
        return IPC_OK;
    }

    proc_t *sender = sender_dequeue(current, src_pid);
    if (sender) {
        msg_copy_out(msg, sender);
        sender->state = PROC_STATE_READY;
        return IPC_OK;
    }

    current->state = PROC_STATE_RECEIVING;
    return IPC_WOULD_BLOCK;
}

int ipc_reply(uint64_t dest_pid, message_t *msg) {
    (void)dest_pid;
    (void)msg;
    return IPC_ERROR;
}
