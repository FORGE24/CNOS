/* kernel/fs/vfs.c - VFS 实现：根挂载位 + ext2 后端 */

#include "fs/vfs.h"
#include "fs/cnos/porting.h"
#include "fs/cnos/cnos_ext2_vol.h"
#include "user_fd.h"

typedef struct {
    int mounted;
    vfs_fstype_t fstype;
} vfs_root_state_t;

static vfs_root_state_t g_root;

static const cnos_vol_t *vfs_active_vol(void) {
    if (!g_root.mounted) {
        return NULL;
    }
    return cnos_vol_current();
}

void vfs_init(void) {
    g_root.mounted = 0;
    g_root.fstype = VFS_FS_NONE;
    cnos_ext2_init();
}

int vfs_mount_root(void) {
    const cnos_vol_t *v = cnos_vol_current();
    if (!v) {
        return VFS_ERR_NOENT;
    }
    g_root.mounted = 1;
    g_root.fstype = VFS_FS_EXT2;
    return VFS_ERR_NONE;
}

int vfs_umount_root(void) {
    user_fd_umount_close_all();
    g_root.mounted = 0;
    g_root.fstype = VFS_FS_NONE;
    return VFS_ERR_NONE;
}

int vfs_is_mounted(void) {
    return g_root.mounted && cnos_vol_current() != NULL;
}

vfs_fstype_t vfs_root_fstype(void) {
    return g_root.fstype;
}

int vfs_format(void) {
    const cnos_vol_t *v = cnos_vol_current();
    if (!v) {
        return VFS_ERR_NOENT;
    }
    if (cnos_ext2_format(v) != 0) {
        return VFS_ERR_IO;
    }
    return VFS_ERR_NONE;
}

int vfs_ls(void) {
    const cnos_vol_t *v = vfs_active_vol();
    if (!v) {
        return VFS_ERR_NOTMOUNTED;
    }
    if (cnos_ext2_ls(v) != 0) {
        return VFS_ERR_IO;
    }
    return VFS_ERR_NONE;
}

int vfs_read_file_range(const char *name, uint32_t offset, void *buf, size_t buf_sz,
                        size_t *out_len) {
    const cnos_vol_t *v = vfs_active_vol();
    if (!v) {
        return VFS_ERR_NOTMOUNTED;
    }
    if (cnos_ext2_read_file_range(v, name, offset, buf, buf_sz, out_len) != 0) {
        return VFS_ERR_IO;
    }
    return VFS_ERR_NONE;
}

int vfs_read_file(const char *name, char *buf, size_t buf_sz, size_t *out_len) {
    return vfs_read_file_range(name, 0u, buf, buf_sz, out_len);
}

int vfs_write_file(const char *name, const char *data, size_t len) {
    const cnos_vol_t *v = vfs_active_vol();
    if (!v) {
        return VFS_ERR_NOTMOUNTED;
    }
    if (cnos_ext2_write_file(v, name, data, len) != 0) {
        return VFS_ERR_IO;
    }
    return VFS_ERR_NONE;
}

int vfs_stat(const char *path, vfs_stat_t *st) {
    const cnos_vol_t *v = vfs_active_vol();
    if (!v) {
        return VFS_ERR_NOTMOUNTED;
    }
    if (!path || !st) {
        return VFS_ERR_IO;
    }
    uint32_t sz = 0;
    if (cnos_ext2_stat_file(v, path, &sz) != 0) {
        return VFS_ERR_NOENT;
    }
    st->ino = 0;
    st->size = sz;
    return VFS_ERR_NONE;
}
