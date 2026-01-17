#include "magisk_interface.h"
#include <dlfcn.h>
#include <unistd.h>
#include <sys/mman.h>
#include <cstring>
#include <cstdio>

namespace ukc {
namespace magisk {

// Magisk 提供的函数指针
typedef int (*magisk_set_memory_rw_t)(void*, size_t);
typedef int (*magisk_set_memory_ro_t)(void*, size_t);
typedef ssize_t (*magisk_read_kernel_memory_t)(uintptr_t, void*, size_t);
typedef ssize_t (*magisk_write_kernel_memory_t)(uintptr_t, const void*, size_t);
typedef uintptr_t (*magisk_kallsyms_lookup_name_t)(const char*);

static magisk_set_memory_rw_t g_magisk_set_memory_rw = nullptr;
static magisk_set_memory_ro_t g_magisk_set_memory_ro = nullptr;
static magisk_read_kernel_memory_t g_magisk_read_kernel_memory = nullptr;
static magisk_write_kernel_memory_t g_magisk_write_kernel_memory = nullptr;
static magisk_kallsyms_lookup_name_t g_magisk_kallsyms_lookup_name = nullptr;
static bool g_magisk_initialized = false;
static bool g_magisk_available = false;

/**
 * 初始化 Magisk 接口
 */
static void init_magisk_interface() {
    if (g_magisk_initialized) return;
    g_magisk_initialized = true;

    // 尝试加载 Magisk 提供的库
    void* magisk_lib = dlopen("libmagisk.so", RTLD_LAZY);
    if (!magisk_lib) {
        // 尝试其他可能的库名
        magisk_lib = dlopen("libmagisk_inject.so", RTLD_LAZY);
    }

    if (!magisk_lib) {
        return;
    }

    // 加载函数指针
    g_magisk_set_memory_rw = (magisk_set_memory_rw_t)dlsym(magisk_lib, "magisk_set_memory_rw");
    g_magisk_set_memory_ro = (magisk_set_memory_ro_t)dlsym(magisk_lib, "magisk_set_memory_ro");
    g_magisk_read_kernel_memory = (magisk_read_kernel_memory_t)dlsym(magisk_lib, "magisk_read_kernel_memory");
    g_magisk_write_kernel_memory = (magisk_write_kernel_memory_t)dlsym(magisk_lib, "magisk_write_kernel_memory");
    g_magisk_kallsyms_lookup_name = (magisk_kallsyms_lookup_name_t)dlsym(magisk_lib, "magisk_kallsyms_lookup_name");

    // 检查是否所有函数都成功加载
    if (g_magisk_set_memory_rw && g_magisk_set_memory_ro &&
        g_magisk_read_kernel_memory && g_magisk_write_kernel_memory) {
        g_magisk_available = true;
    }
}

bool is_magisk_available() {
    init_magisk_interface();
    return g_magisk_available;
}

int magisk_set_memory_rw(void* addr, size_t size) {
    init_magisk_interface();
    if (!g_magisk_set_memory_rw) {
        return -1;
    }
    return g_magisk_set_memory_rw(addr, size);
}

int magisk_set_memory_ro(void* addr, size_t size) {
    init_magisk_interface();
    if (!g_magisk_set_memory_ro) {
        return -1;
    }
    return g_magisk_set_memory_ro(addr, size);
}

ssize_t magisk_read_kernel_memory(uintptr_t addr, void* buf, size_t size) {
    init_magisk_interface();
    if (!g_magisk_read_kernel_memory) {
        return -1;
    }
    return g_magisk_read_kernel_memory(addr, buf, size);
}

ssize_t magisk_write_kernel_memory(uintptr_t addr, const void* buf, size_t size) {
    init_magisk_interface();
    if (!g_magisk_write_kernel_memory) {
        return -1;
    }
    return g_magisk_write_kernel_memory(addr, buf, size);
}

uintptr_t magisk_kallsyms_lookup_name(const char* name) {
    init_magisk_interface();
    if (!g_magisk_kallsyms_lookup_name) {
        return 0;
    }
    return g_magisk_kallsyms_lookup_name(name);
}

int SafeMemoryModifier::modify_kernel_memory(uintptr_t addr, const void* data, size_t size) {
    // 第1步：设置内存为可写
    if (magisk_set_memory_rw((void*)addr, size) != 0) {
        return -1;
    }

    // 第2步：写入数据
    ssize_t written = magisk_write_kernel_memory(addr, data, size);
    if (written != (ssize_t)size) {
        magisk_set_memory_ro((void*)addr, size);
        return -1;
    }

    // 第3步：恢复为只读（避免被检测）
    if (magisk_set_memory_ro((void*)addr, size) != 0) {
        return -1;
    }

    return 0;
}

ssize_t SafeMemoryModifier::read_kernel_memory(uintptr_t addr, void* buf, size_t size) {
    return magisk_read_kernel_memory(addr, buf, size);
}

} // namespace magisk
} // namespace ukc
