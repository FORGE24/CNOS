cmake_minimum_required(VERSION 3.16)

# 首次配置须指定工具链，例如：
#   cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/x86_64-elf.cmake
# 或使用：cmake --preset default（见 CMakePresets.json）
# 已有 build/ 后只需：cmake -B build 或 cmake --build build，勿重复传工具链，否则 CMake 会警告变量未使用。
if(CMAKE_TOOLCHAIN_FILE)
    message(STATUS "CNOS: CMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}")
endif()

project(CNOS LANGUAGES C ASM_NASM)

set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)

set(KERNEL_INCLUDE ${CMAKE_SOURCE_DIR}/kernel)
set(KERNEL_FS_CNOS ${CMAKE_SOURCE_DIR}/kernel/fs/cnos)
set(KERNEL_FS_INCLUDE ${CMAKE_SOURCE_DIR}/kernel/fs/include)
set(KERNEL_FS_LIB ${CMAKE_SOURCE_DIR}/kernel/fs/lib)
set(KERNEL_FS_EXT2FS ${CMAKE_SOURCE_DIR}/kernel/fs/lib/ext2fs)
set(KERNEL_FS_CNAF ${CMAKE_SOURCE_DIR}/kernel/fs/cnaf)

# e2fsprogs 整树在 kernel/fs/lib 等目录；CNOS 只链接下列 .c，按需再追加 lib/ext2fs/*.c（见 kernel/fs/README）

set(KERNEL_ASM
    kernel/entry.asm
    kernel/interrupts.asm
)

set(KERNEL_C
    kernel/main.c
    kernel/multiboot.c
    kernel/pmm.c
    kernel/vmm.c
    kernel/idt.c
    kernel/isr.c
    kernel/shell.c
    kernel/ipc.c
    kernel/fs/vfs.c
    kernel/fs/cnos/cnos_alloc.c
    kernel/fs/cnos/cnos_ext2.c
    kernel/fs/cnaf/cnaf_validate.c
    kernel/drivers/vga.c
    kernel/drivers/serial.c
    kernel/drivers/pci.c
    kernel/process.c
)

add_executable(kernel.elf ${KERNEL_ASM} ${KERNEL_C})

target_include_directories(kernel.elf PRIVATE
    ${KERNEL_INCLUDE}
    ${KERNEL_FS_CNOS}
    ${KERNEL_FS_CNAF}
    ${KERNEL_FS_INCLUDE}
    ${KERNEL_FS_LIB}
    ${KERNEL_FS_EXT2FS}
)

target_compile_options(kernel.elf PRIVATE
    $<$<COMPILE_LANGUAGE:C>:-ffreestanding;-O2;-Wall;-Wextra;-m64;-fno-stack-protector;-nostdlib;-fno-builtin>
)

target_link_options(kernel.elf PRIVATE
    -nostdlib
    "-T${CMAKE_SOURCE_DIR}/linker.ld"
)

set_target_properties(kernel.elf PROPERTIES
    LINK_DEPENDS ${CMAKE_SOURCE_DIR}/linker.ld
)

find_program(GRUB_MKRESCUE NAMES grub-mkrescue grub2-mkrescue)

if(GRUB_MKRESCUE)
    add_custom_command(
        OUTPUT ${CMAKE_BINARY_DIR}/cnos.iso
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/iso/boot
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:kernel.elf>
                ${CMAKE_SOURCE_DIR}/iso/boot/kernel.elf
        COMMAND ${GRUB_MKRESCUE} -o ${CMAKE_BINARY_DIR}/cnos.iso ${CMAKE_SOURCE_DIR}/iso
        DEPENDS kernel.elf ${CMAKE_SOURCE_DIR}/iso/boot/grub/grub.cfg
        COMMENT "生成 cnos.iso (GRUB2)"
        VERBATIM
    )
    add_custom_target(cnos_iso ALL DEPENDS ${CMAKE_BINARY_DIR}/cnos.iso)
else()
    message(WARNING "未找到 grub-mkrescue / grub2-mkrescue，跳过 ISO；仅生成 kernel.elf")
endif()

find_program(QEMU_SYSTEM_X86_64 qemu-system-x86_64)
if(QEMU_SYSTEM_X86_64 AND GRUB_MKRESCUE)
    add_custom_target(run
        COMMAND ${QEMU_SYSTEM_X86_64}
            -cdrom ${CMAKE_BINARY_DIR}/cnos.iso
            -m 128M
            -serial stdio
        DEPENDS cnos_iso
        USES_TERMINAL
    )
endif()
