#!/bin/bash
# 从 GNU 源码构建 x86_64-elf 交叉 binutils + gcc（裸机 / 无 libc）。
# 用法: scripts/build-x86_64-elf-toolchain.sh <安装前缀>
# 环境变量 GNU_MIRROR 可指向镜像（默认 https://ftp.gnu.org/gnu）。
set -e

TARGET=x86_64-elf
BINUTILS_VER=2.42
GCC_VER=14.2.0
GNU_MIRROR="${GNU_MIRROR:-https://ftp.gnu.org/gnu}"

if [ -z "${1:-}" ]; then
  echo "用法: $0 <安装前缀>" >&2
  exit 1
fi

PREFIX="$1"
mkdir -p "$PREFIX"
PREFIX="$(cd "$PREFIX" && pwd)"

if [ -x "$PREFIX/bin/${TARGET}-gcc" ] && [ -x "$PREFIX/bin/${TARGET}-ld" ]; then
  echo "已存在 ${TARGET}-gcc / ${TARGET}-ld（$PREFIX），跳过编译。"
  exit 0
fi

ROOT="$(pwd)"
SRCDIR="$ROOT/build-tmp"
rm -rf "$SRCDIR"
mkdir -p "$SRCDIR"
cd "$SRCDIR"

fetch() {
  local url="$1" out="$2"
  if command -v wget >/dev/null 2>&1; then
    wget -q -O "$out" "$url"
  else
    curl -fsSL -o "$out" "$url"
  fi
}

echo "=== 构建目录: $SRCDIR，安装到: $PREFIX ==="

fetch "$GNU_MIRROR/binutils/binutils-${BINUTILS_VER}.tar.xz" "binutils-${BINUTILS_VER}.tar.xz"
fetch "$GNU_MIRROR/gcc/gcc-${GCC_VER}/gcc-${GCC_VER}.tar.xz" "gcc-${GCC_VER}.tar.xz"

tar -xf "binutils-${BINUTILS_VER}.tar.xz"
tar -xf "gcc-${GCC_VER}.tar.xz"

# --- binutils ---
mkdir -p build-binutils && cd build-binutils
"../binutils-${BINUTILS_VER}/configure" \
  --target="$TARGET" \
  --prefix="$PREFIX" \
  --with-sysroot \
  --disable-nls \
  --disable-werror
make -j"$(nproc)"
make install
cd "$SRCDIR"

export PATH="$PREFIX/bin:$PATH"

# --- gcc：先用官方脚本拉取 GMP/MPFR/MPC 到源码树，避免仅依赖系统 -dev 包时 configure 判版本失败 ---
cd "gcc-${GCC_VER}"
./contrib/download_prerequisites
cd "$SRCDIR"

mkdir -p build-gcc && cd build-gcc
"../gcc-${GCC_VER}/configure" \
  --target="$TARGET" \
  --prefix="$PREFIX" \
  --disable-nls \
  --enable-languages=c \
  --without-headers \
  --disable-multilib \
  --disable-threads \
  --with-gnu-as \
  --with-gnu-ld

make -j"$(nproc)" all-gcc all-target-libgcc
make install-gcc install-target-libgcc
cd "$SRCDIR"

cd "$ROOT"
rm -rf "$SRCDIR"

echo "=== 工具链已安装到 $PREFIX ==="
