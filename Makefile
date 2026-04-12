# 薄封装：实际构建由 CMake 完成
BUILD_DIR ?= build
TOOLCHAIN ?= $(CURDIR)/cmake/x86_64-elf.cmake

.PHONY: all clean configure run

all: configure
	cmake --build $(BUILD_DIR)

# 已有 build/CMakeCache 时不再传工具链，避免 “CMAKE_TOOLCHAIN_FILE was not used” 警告
configure:
	@test -f $(BUILD_DIR)/CMakeCache.txt && cmake -B $(BUILD_DIR) || \
		cmake -B $(BUILD_DIR) -DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN)

clean:
	@cmake --build $(BUILD_DIR) --target clean 2>/dev/null || true
	rm -rf $(BUILD_DIR)

run: all
	qemu-system-x86_64 -cdrom $(BUILD_DIR)/cnos.iso -m 128M -serial stdio
