/*
 * ChaserOS 专用 io_manager：内存盘（或日后块设备），无 pthread、无 Unix fd。
 */

#ifndef CHASEROS_IO_H
#define CHASEROS_IO_H

#include "et/com_err.h"
#include "ext2_io.h"

extern io_manager chaseros_io_manager;

void chaseros_ramdisk_attach(void *base, size_t nbytes);

/* 自检：打开、set_blksize(1024)、read_blk(0)，成功返回 0 */
errcode_t chaseros_io_selftest(void);

#endif
