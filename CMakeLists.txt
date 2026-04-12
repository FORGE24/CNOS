cmake_minimum_required(VERSION 3.16)

# 未指定工具链时仍可配置；构建内核前请用 -DCMAKE_TOOLCHAIN_FILE=cmake/x86_64-elf.cmake
project(CNOS LANGUAGES C ASM_NASM)

set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)

set(KERNEL_INCLUDE ${CMAKE_SOURCE_DIR}/kernel)

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
    kernel/drivers/vga.c
    kernel/drivers/serial.c
    kernel/drivers/pci.c
    kernel/process.c
    kernel/drivers/graphics.c
    kernel/drivers/font.c
    kernel/drivers/gui.c
    kernel/drivers/sheet.c
    kernel/drivers/mouse.c
)

add_executable(kernel.elf ${KERNEL_ASM} ${KERNEL_C})

target_include_directories(kernel.elf PRIVATE ${KERNEL_INCLUDE})

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
