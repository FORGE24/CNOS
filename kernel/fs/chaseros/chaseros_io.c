#include "fs/chaseros/config.h"
#include "fs/chaseros/porting.h"
#include "fs/chaseros/chaseros_io.h"

#include <stddef.h>

#include "ext2fs/ext2_err.h"

static void chaseros_memcpy(void *dst, const void *src, size_t n) {
    unsigned char *d = dst;
    const unsigned char *s = src;
    while (n--) {
        *d++ = *s++;
    }
}

static void chaseros_memset(void *dst, int c, size_t n) {
    unsigned char *d = dst;
    unsigned char v = (unsigned char)c;
    while (n--) {
        *d++ = v;
    }
}

struct chaseros_ramdisk_data {
    void *base;
    size_t nbytes;
};

static void *g_ramdisk_base;
static size_t g_ramdisk_len;

void chaseros_ramdisk_attach(void *base, size_t nbytes) {
    g_ramdisk_base = base;
    g_ramdisk_len = nbytes;
}

static errcode_t chaseros_close(io_channel channel) {
    if (!channel || channel->magic != EXT2_ET_MAGIC_IO_CHANNEL) {
        return EXT2_ET_MAGIC_IO_CHANNEL;
    }
    channel->magic = 0;
    if (channel->name) {
        chaseros_free(channel->name);
        channel->name = NULL;
    }
    if (channel->private_data) {
        chaseros_free(channel->private_data);
        channel->private_data = NULL;
    }
    chaseros_free(channel);
    return 0;
}

static errcode_t chaseros_set_blksize(io_channel channel, int blksize) {
    if (blksize <= 0 || (blksize & (blksize - 1))) {
        return EXT2_ET_INVALID_ARGUMENT;
    }
    channel->block_size = blksize;
    return 0;
}

static errcode_t chaseros_read_blk(io_channel channel, unsigned long block, int count,
                               void *data) {
    struct chaseros_ramdisk_data *d = channel->private_data;
    unsigned bs = (unsigned)channel->block_size;

    if (!d || !data || count <= 0 || bs == 0) {
        return EXT2_ET_INVALID_ARGUMENT;
    }
    unsigned long long off = (unsigned long long)block * bs;
    unsigned long long need = (unsigned long long)(unsigned)count * bs;
    if (off + need > (unsigned long long)d->nbytes) {
        return EXT2_ET_SHORT_READ;
    }
    chaseros_memcpy(data, (const unsigned char *)d->base + off, (size_t)need);
    return 0;
}

static errcode_t chaseros_write_blk(io_channel channel, unsigned long block, int count,
                                const void *data) {
    struct chaseros_ramdisk_data *d = channel->private_data;
    unsigned bs = (unsigned)channel->block_size;

    if (!d || !data || count <= 0 || bs == 0) {
        return EXT2_ET_INVALID_ARGUMENT;
    }
    unsigned long long off = (unsigned long long)block * bs;
    unsigned long long need = (unsigned long long)(unsigned)count * bs;
    if (off + need > (unsigned long long)d->nbytes) {
        return EXT2_ET_SHORT_WRITE;
    }
    chaseros_memcpy((unsigned char *)d->base + off, data, (size_t)need);
    return 0;
}

static errcode_t chaseros_flush(io_channel channel) {
    (void)channel;
    return 0;
}

static errcode_t chaseros_open(const char *name, int flags, io_channel *channel) {
    (void)flags;
    io_channel io;
    struct chaseros_ramdisk_data *pd;
    size_t nl;
    errcode_t err;

    (void)name;
    if (!channel || g_ramdisk_len == 0 || !g_ramdisk_base) {
        return EXT2_ET_INVALID_ARGUMENT;
    }

    io = chaseros_malloc(sizeof(struct struct_io_channel));
    if (!io) {
        return EXT2_ET_NO_MEMORY;
    }
    chaseros_memset(io, 0, sizeof(struct struct_io_channel));
    io->magic = EXT2_ET_MAGIC_IO_CHANNEL;
    io->manager = chaseros_io_manager;
    io->block_size = 1024;
    io->refcount = 1;

    nl = 0;
    if (name) {
        const char *p = name;
        while (*p++) {
            nl++;
        }
    }
    io->name = chaseros_malloc(nl + 1);
    if (!io->name) {
        err = EXT2_ET_NO_MEMORY;
        goto bad_io;
    }
    if (name && nl) {
        chaseros_memcpy(io->name, name, nl + 1);
    } else {
        io->name[0] = '\0';
    }

    pd = chaseros_malloc(sizeof(struct chaseros_ramdisk_data));
    if (!pd) {
        err = EXT2_ET_NO_MEMORY;
        goto bad_name;
    }
    pd->base = g_ramdisk_base;
    pd->nbytes = g_ramdisk_len;
    io->private_data = pd;
    *channel = io;
    return 0;

bad_name:
    chaseros_free(io->name);
bad_io:
    chaseros_free(io);
    return err;
}

static struct struct_io_manager chaseros_mgr_storage = {
    .magic = EXT2_ET_MAGIC_IO_MANAGER,
    .name = "chaseros_ramdisk",
    .open = chaseros_open,
    .close = chaseros_close,
    .set_blksize = chaseros_set_blksize,
    .read_blk = chaseros_read_blk,
    .write_blk = chaseros_write_blk,
    .flush = chaseros_flush,
};

io_manager chaseros_io_manager = &chaseros_mgr_storage;

errcode_t chaseros_io_selftest(void) {
    static unsigned char buf[4096] __attribute__((aligned(8)));
    io_channel ch = NULL;
    errcode_t e;
    unsigned char out[1024];

    chaseros_memset(buf, 0xCD, sizeof buf);
    buf[0] = 0x11;
    buf[1] = 0x22;

    chaseros_ramdisk_attach(buf, sizeof buf);
    e = chaseros_io_manager->open("chaseros_selftest", 0, &ch);
    if (e) {
        return e;
    }
    e = io_channel_set_blksize(ch, 1024);
    if (e) {
        chaseros_io_manager->close(ch);
        return e;
    }
    e = io_channel_read_blk(ch, 0, 1, out);
    if (e) {
        chaseros_io_manager->close(ch);
        return e;
    }
    if (out[0] != 0x11 || out[1] != 0x22) {
        chaseros_io_manager->close(ch);
        return EXT2_ET_SHORT_READ;
    }
    return chaseros_io_manager->close(ch);
}
