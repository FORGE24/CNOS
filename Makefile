# CNOS 项目编译脚本 (Makefile)

# 编译器与工具链定义
CC = x86_64-elf-gcc
LD = x86_64-elf-ld
NASM = nasm
OBJCOPY = x86_64-elf-objcopy

# 编译选项
# -ffreestanding: 告知编译器不假设标准库存在
# -m64: 生成 64 位代码
# -fno-stack-protector: 禁用栈保护（内核初期不需要）
# -nostdlib: 不链接标准库
CFLAGS = -ffreestanding -O2 -Wall -Wextra -m64 -fno-stack-protector -nostdlib -fno-builtin
LDFLAGS = -T linker.ld -nostdlib

# 源文件路径
BOOT_SRC = boot/boot.asm
KERNEL_ENTRY = kernel/entry.asm
KERNEL_C_SRC = kernel/main.c

# 编译生成的目标文件路径
BOOT_BIN = build/boot.bin
KERNEL_BIN = build/kernel.bin
KERNEL_ELF = build/kernel.elf
CNOS_IMG = cnos.img

# 对象文件列表
OBJS = build/entry.o build/main.o build/pmm.o build/vmm.o build/idt.o build/isr.o build/interrupts.o build/shell.o build/ipc.o \
       build/vga.o build/serial.o build/pci.o build/process.o build/graphics.o build/font.o build/gui.o build/sheet.o build/mouse.o

# 默认目标
all: $(CNOS_IMG)

# 生成磁盘镜像
$(CNOS_IMG): $(BOOT_BIN) $(KERNEL_BIN)
	@echo "正在创建磁盘镜像..."
	dd if=/dev/zero of=$(CNOS_IMG) bs=512 count=2880
	dd if=$(BOOT_BIN) of=$(CNOS_IMG) conv=notrunc
	dd if=$(KERNEL_BIN) of=$(CNOS_IMG) seek=1 conv=notrunc

# 编译引导扇区
$(BOOT_BIN): $(BOOT_SRC)
	@mkdir -p build
	$(NASM) -f bin $< -o $@

# 将 ELF 格式内核转换为二进制格式
$(KERNEL_BIN): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@

# 链接内核
$(KERNEL_ELF): $(OBJS) linker.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJS)

# 编译内核入口汇编
build/entry.o: $(KERNEL_ENTRY)
	@mkdir -p build
	$(NASM) -f elf64 $< -o $@

# 编译内核 C 语言代码
build/main.o: $(KERNEL_C_SRC)
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/pmm.o: kernel/pmm.c kernel/pmm.h
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/vmm.o: kernel/vmm.c kernel/vmm.h kernel/pmm.h
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/idt.o: kernel/idt.c kernel/idt.h
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/isr.o: kernel/isr.c
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/interrupts.o: kernel/interrupts.asm
	@mkdir -p build
	$(NASM) -f elf64 $< -o $@

build/shell.o: kernel/shell.c kernel/shell.h
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/ipc.o: kernel/ipc.c kernel/ipc.h kernel/process.h
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/vga.o: kernel/drivers/vga.c kernel/drivers/vga.h
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/serial.o: kernel/drivers/serial.c kernel/drivers/serial.h
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/pci.o: kernel/drivers/pci.c kernel/drivers/pci.h
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/graphics.o: kernel/drivers/graphics.c kernel/drivers/graphics.h kernel/drivers/font.h
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/font.o: kernel/drivers/font.c kernel/drivers/font.h
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/gui.o: kernel/drivers/gui.c kernel/drivers/gui.h kernel/drivers/graphics.h
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/sheet.o: kernel/drivers/sheet.c kernel/drivers/sheet.h
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/mouse.o: kernel/drivers/mouse.c kernel/drivers/mouse.h
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

build/process.o: kernel/process.c kernel/process.h
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

# 清理编译生成的文件
clean:
	rm -rf build $(CNOS_IMG)

# 在 QEMU 中运行
run: all
	qemu-system-x86_64 -fda $(CNOS_IMG) -m 64M
