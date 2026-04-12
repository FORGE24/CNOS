#!/bin/bash

# CNOS 虚拟机运行脚本

# 检查依赖项
if ! command -v qemu-system-x86_64 &> /dev/null
then
    echo "错误: 未找到 qemu-system-x86_64，请先安装 QEMU。"
    exit 1
fi

# 检查磁盘镜像是否存在
if [ ! -f "cnos.img" ]; then
    echo "正在编译项目..."
    make all
fi

echo "正在启动 CNOS 64位微内核..."

# 启动 QEMU
# -fda cnos.img: 使用软盘镜像
# -m 128M: 分配 128MB 内存
# -no-reboot: 发生错误时不自动重启
# -serial stdio: 将串口输出重定向到终端 (如果以后实现串口驱动)
qemu-system-x86_64 \
    -fda cnos.img \
    -m 128M \
    -no-reboot \
    -no-shutdown \
    -d cpu_reset \
    -monitor stdio
