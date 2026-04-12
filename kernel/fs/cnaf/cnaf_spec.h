/*
 * CNAF / CNAFL — 规范与路线图（CNOS Application Files & Libs）
 *
 * 本文档以头文件形式固定命名与布局约定，便于内核、加载器与宿主工具链共用一个“真相源”。
 * 实现顺序：先只读解析与校验 → 再与 VFS/块设备挂钩 → 最后用户态加载与链接。
 */

#ifndef CNOS_CNAF_SPEC_H
#define CNOS_CNAF_SPEC_H

#include <stdint.h>

/* -------------------------------------------------------------------------- */
/* 命名                                                                        */
/* -------------------------------------------------------------------------- */

/*
 * CNAF — CNOS Application File
 *   一个可部署的「应用单元」：元数据 + 资源 + 可加载映像（通常为 ELF64，CNOS 子集 ABI）。
 *
 * CNAFL — CNOS Application Files Library
 *   面向应用的可分发库单元：元数据 + 一个或多个库映像（静态归档或动态共享占位，由版本迭代定义）。
 *
 * 关系：CNAF 可在清单中声明对若干 CNAFL 的依赖（名称 + 兼容 ABI 范围）；CNAFL 不依赖 CNAF。
 */

/* 建议文件扩展名（工具链与资源管理器约定，内核可仅认魔数） */
#define CNAF_FILE_EXT  ".cnaf"
#define CNAFL_FILE_EXT ".cnafl"

/* 根卷上的约定路径（与具体 FS 无关；挂载根后由 VFS 解析） */
#define CNAF_SYS_APPS_DIR   "/System/Apps"
#define CNAF_SYS_LIB_DIR    "/System/Lib"
#define CNAF_USER_APPS_DIR  "/Home/Apps"

/* -------------------------------------------------------------------------- */
/* 文件头（磁盘 / 内存映像共用；小端）                                           */
/* -------------------------------------------------------------------------- */

#define CNAF_MAGIC_U32 0x46414E43u /* 'C' 'N' 'A' 'F' 小端序 */

#define CNAF_FMT_MAJOR 0u
#define CNAF_FMT_MINOR 1u

/*
 * CNAF/CNAFL 共用同一文件头前缀；类型字段区分用途。
 */
typedef enum {
    CNAF_PAYLOAD_APP  = 1,
    CNAF_PAYLOAD_LIB  = 2,
} cnaf_payload_kind_t;

typedef struct cnaf_file_header {
    uint32_t magic;           /* CNAF_MAGIC_U32 */
    uint16_t fmt_major;
    uint16_t fmt_minor;
    uint32_t payload_kind;    /* cnaf_payload_kind_t */
    uint32_t header_bytes;    /* 含本头及后续固定元数据；字节对齐见下 */
    uint32_t flags;           /* 保留；如：是否签名、是否压缩 */
    uint8_t  id[16];          /* UUID 或构建 id，全 0 表示未设置 */
} cnaf_file_header_t;

#define CNAF_HEADER_ALIGN 8u

/*
 * 头之后按顺序排列若干「节」（section），由目录表定位（下一版在 fmt_minor 中冻结）：
 *   - MANIFEST：UTF-8 文本或极简键值（应用名、入口符号、所需 CNAFL 列表）
 *   - RESOURCES：可选；图标、本地化等
 *   - IMAGE：可加载段（ELF 或 CNOS 自定义 LOB）
 *
 * v0.1 仅要求：可读魔数、fmt、payload_kind；目录表可为空（单 IMAGE 紧跟在固定偏移）。
 */

/* -------------------------------------------------------------------------- */
/* CNAFL 特有（与 CNAF 共用文件头，payload_kind == CNAF_PAYLOAD_LIB）             */
/* -------------------------------------------------------------------------- */

/*
 * 计划字段（实现时写入扩展头或 MANIFEST）：
 *   - lib_name：如 "cnui", "net"
 *   - abi_major / abi_minor：与内核/加载器约定的接口版本
 *   - soname 等价物：供依赖解析
 */

/* -------------------------------------------------------------------------- */
/* 错误码（加载器 / 内核解析共用）                                               */
/* -------------------------------------------------------------------------- */

typedef enum {
    CNAF_OK = 0,
    CNAF_ERR_MAGIC = 1,
    CNAF_ERR_VERSION = 2,
    CNAF_ERR_TRUNCATED = 3,
    CNAF_ERR_KIND = 4,
    /* 预留 */
} cnaf_err_t;

/* -------------------------------------------------------------------------- */
/* 实施路线图（非枚举，仅供维护者对照）                                          */
/* -------------------------------------------------------------------------- */

/*
 * Phase A — 规范冻结：魔数、fmt、header 大小、payload_kind；内核 cnaf_probe_header；
 *           宿主工具 `cnaf-dump` 可后续接同一头布局。
 * Phase B — 与 VFS 结合：从 ext2 路径打开 .cnaf/.cnafl，内核或 init 校验头。
 * Phase C — 用户态加载器：解析 IMAGE 节，按 PMM/VMM 映射 ELF PT_LOAD。
 * Phase D — CNAFL 依赖：解析 MANIFEST 依赖表，按版本解析共享库；符号解析与重定位。
 *
 * 与 libext2fs：仅通过 VFS 路径与 io_channel 读文件；CNAF 不依赖磁盘上的 inode 布局。
 */

#endif /* CNOS_CNAF_SPEC_H */
