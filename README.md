# CNOS

CNOS（CN OS）是基于 **64 位微内核** 的实验性操作系统，通过 **GRUB2 + Multiboot2** 从 ISO 启动，当前以 **VGA 文本模式 + 串口** 作为控制台，正在接入 **e2fsprogs / libext2fs** 子集与自有 **VFS / CNAF** 格式。

## 特性（概览）

- **引导**：Multiboot2 头位于 `.text` 最前；可选用 `gfxpayload=text` 保持 VGA 文本显存可见。
- **控制台**：`0xB8000` 文本缓冲 + COM1 镜像输出；Shell 经 PS/2 键盘（IRQ1）。
- **内存**：物理页管理（PMM）、页表（VMM）基础框架。
- **文件系统**：VFS 占位；`kernel/fs` 内含裁剪后的 **e2fsprogs** 源码树供对接 ext2/3/4（仅部分文件会编入内核，见 `kernel/fs/README`）。
- **应用包格式**：CNAF / CNAFL 规范与头校验（`kernel/fs/cnaf/`）。

## 依赖

| 工具 | 用途 |
|------|------|
| CMake ≥ 3.16 | 构建 |
| NASM | 内核汇编（`elf64`） |
| `x86_64-elf-gcc` + binutils | 裸机交叉编译（无 libc） |
| xorriso、`grub-mkrescue` | 生成 `cnos.iso`（可选；无则只生成 `kernel.elf`） |
| QEMU `qemu-system-x86_64` | 本地运行 ISO |

**Fedora**：若仓库无 `x86_64-elf-gcc`，需自装交叉工具链（常见路径 `~/opt/cross/bin`），或设置环境变量 `CNOS_X86_64_ELF_GCC` 指向编译器。

**Ubuntu / Debian** 示例：

```bash
sudo apt install cmake nasm gcc-x86-64-elf binutils-x86-64-elf xorriso grub-common grub-pc-bin qemu-system-x86
```

## 构建

在仓库根目录执行：

```bash
cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/x86_64-elf.cmake
cmake --build build
```

产物：

- `build/kernel.elf` — 内核 ELF  
- `build/cnos.iso` — 可启动镜像（需已安装 GRUB/xorriso）

若已配置过 `build/`且未换工具链，可直接：

```bash
cmake -B build
cmake --build build
```

交叉编译器不在默认路径时：

```bash
export CNOS_X86_64_ELF_GCC=/path/to/x86_64-elf-gcc
cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/x86_64-elf.cmake
cmake --build build
```

或使用 Makefile 封装：

```bash
make          # 首次自动带工具链文件配置
make clean    # 删除 build 目录
```

等价预设：`cmake --preset default`（见 `CMakePresets.json`）。

## 运行（QEMU）

```bash
qemu-system-x86_64 -cdrom build/cnos.iso -m 128M -serial stdio
```

或：

```bash
make run
```

串口与 VGA 会同时输出；若只看终端，使用 `-serial stdio` 即可。

## 目录速览

| 路径 | 说明 |
|------|------|
| `kernel/` | 内核源码（C / NASM） |
| `kernel/fs/cnos/` | e2fsprogs 适配（`config.h`、`cnos_alloc` 等） |
| `kernel/fs/cnaf/` | CNAF/CNAFL 头解析 |
| `kernel/fs/lib/ext2fs/` 等 | e2fsprogs 库源码（按需加入 CMake） |
| `cmake/x86_64-elf.cmake` | 裸机工具链 |
| `iso/boot/grub/grub.cfg` | GRUB 菜单（`multiboot2` 加载内核） |
| `.github/workflows/` | CI 与 Release（推送 `v*` tag 发布 ISO） |

更细的 **kernel/fs** 说明见 `kernel/fs/README`。

## 自动化（GitHub Actions）

- **CI**：推送/PR 至 `main` 时编译 `kernel.elf` 与 `cnos.iso`，并上传 Artifact。  
- **Release**：推送 `v*` 标签（如 `v0.1.0`）时构建并发布 `cnos-<tag>.iso` 与 `kernel-<tag>.elf`。

## 语言与界面

- 控制台与 Shell 消息为 **英文** 为主；文档与注释可为 **中文**。  
- 图形界面（帧缓冲 GUI）当前默认不编入内核，便于稳定使用文本终端。

## 许可

上游 **e2fsprogs** 等文件遵循其原有 GPL/LGPL 等声明；仓库内 CNOS 新增文件以项目整体许可为准（若未单独声明，请与维护者确认）。

---

*后续更新：ext2 挂载、CNAF 加载器、用户态等。*
