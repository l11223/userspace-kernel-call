#include "skroot_interface.h"
#include <dlfcn.h>
#include <cstring>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <asm/ptrace.h>

namespace ukc {
namespace skroot {

// 666 Root 管理器库句柄
static void* root_handle = nullptr;
static bool initialized = false;

// Root 管理器函数指针
typedef int (*root_init_t)();
typedef int (*root_call_kernel_t)(uintptr_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
typedef int (*root_read_mem_t)(uintptr_t, void*, size_t);
typedef int (*root_write_mem_t)(uintptr_t, const void*, size_t);

static root_init_t root_init = nullptr;
static root_call_kernel_t root_call_kernel = nullptr;
static root_read_mem_t root_read_mem = nullptr;
static root_write_mem_t root_write_mem = nullptr;

/**
 * 尝试加载 Root 管理器库（666 或其他）
 */
static Result<bool> load_root_library() {
    if (root_handle != nullptr) {
        return Result<bool>::success(true);
    }

    // 尝试加载各种 Root 管理器库
    const char* root_paths[] = {
        "/system/lib64/lib666.so",
        "/system/lib/lib666.so",
        "/data/local/tmp/lib666.so",
        "/system/lib64/libroot.so",
        "/system/lib/libroot.so",
        "/data/local/tmp/libroot.so",
        nullptr
    };

    for (int i = 0; root_paths[i] != nullptr; i++) {
        root_handle = dlopen(root_paths[i], RTLD_LAZY);
        if (root_handle != nullptr) {
            break;
        }
    }

    // 如果没有找到库，使用直接系统调用方式
    if (root_handle == nullptr) {
        // 检查是否有 Root 权限
        if (getuid() == 0) {
            return Result<bool>::success(true);
        }
        return Result<bool>::error("No Root manager library found and no Root privileges");
    }

    // 加载函数指针
    root_init = (root_init_t)dlsym(root_handle, "root_init");
    root_call_kernel = (root_call_kernel_t)dlsym(root_handle, "root_call_kernel");
    root_read_mem = (root_read_mem_t)dlsym(root_handle, "root_read_mem");
    root_write_mem = (root_write_mem_t)dlsym(root_handle, "root_write_mem");

    return Result<bool>::success(true);
}

Result<bool> initialize_skroot() {
    auto load_result = load_root_library();
    if (!load_result.isSuccess()) {
        return load_result;
    }

    // 如果有 Root 管理器库，初始化它
    if (root_init != nullptr) {
        int ret = root_init();
        if (ret != 0) {
            return Result<bool>::error("Root manager initialization failed with code: " + std::to_string(ret));
        }
    }

    initialized = true;
    return Result<bool>::success(true);
}

Result<bool> is_skroot_available() {
    auto load_result = load_root_library();
    if (!load_result.isSuccess()) {
        return Result<bool>::success(false);
    }

    // 检查是否有 Root 权限或 Root 管理器
    if (getuid() == 0) {
        return Result<bool>::success(true);
    }

    if (root_init != nullptr) {
        int ret = root_init();
        return Result<bool>::success(ret == 0);
    }

    return Result<bool>::success(false);
}

Result<uint64_t> call_kernel_function(
    uintptr_t kernel_func_addr,
    const uint64_t* args,
    size_t arg_count
) {
    if (!initialized) {
        return Result<uint64_t>::error("Root system not initialized");
    }

    if (arg_count > 6) {
        return Result<uint64_t>::error("Too many arguments (max 6)");
    }

    // 如果有 Root 管理器库，使用它
    if (root_call_kernel != nullptr) {
        uint64_t arg0 = (arg_count > 0) ? args[0] : 0;
        uint64_t arg1 = (arg_count > 1) ? args[1] : 0;
        uint64_t arg2 = (arg_count > 2) ? args[2] : 0;
        uint64_t arg3 = (arg_count > 3) ? args[3] : 0;
        uint64_t arg4 = (arg_count > 4) ? args[4] : 0;
        uint64_t arg5 = (arg_count > 5) ? args[5] : 0;

        int ret = root_call_kernel(kernel_func_addr, arg0, arg1, arg2, arg3, arg4, arg5);
        
        if (ret < 0) {
            return Result<uint64_t>::error("Kernel call failed with code: " + std::to_string(ret));
        }

        return Result<uint64_t>::success((uint64_t)ret);
    }

    // 如果有 Root 权限，可以直接调用
    if (getuid() == 0) {
        // 这里需要 ARM64 汇编来调用内核函数
        // 暂时返回错误，需要实现 ARM64 汇编调用
        return Result<uint64_t>::error("Direct kernel call requires ARM64 assembly implementation");
    }

    return Result<uint64_t>::error("No Root manager available");
}

Result<size_t> read_kernel_memory(
    uintptr_t kernel_addr,
    uint8_t* buffer,
    size_t size
) {
    if (!initialized) {
        return Result<size_t>::error("Root system not initialized");
    }

    if (buffer == nullptr || size == 0) {
        return Result<size_t>::error("Invalid buffer or size");
    }

    // 如果有 Root 管理器库，使用它
    if (root_read_mem != nullptr) {
        int ret = root_read_mem(kernel_addr, buffer, size);
        
        if (ret < 0) {
            return Result<size_t>::error("Read memory failed with code: " + std::to_string(ret));
        }

        return Result<size_t>::success((size_t)ret);
    }

    return Result<size_t>::error("No Root manager available for memory read");
}

Result<size_t> write_kernel_memory(
    uintptr_t kernel_addr,
    const uint8_t* data,
    size_t size
) {
    if (!initialized) {
        return Result<size_t>::error("Root system not initialized");
    }

    if (data == nullptr || size == 0) {
        return Result<size_t>::error("Invalid data or size");
    }

    // 如果有 Root 管理器库，使用它
    if (root_write_mem != nullptr) {
        int ret = root_write_mem(kernel_addr, (void*)data, size);
        
        if (ret < 0) {
            return Result<size_t>::error("Write memory failed with code: " + std::to_string(ret));
        }

        return Result<size_t>::success((size_t)ret);
    }

    return Result<size_t>::error("No Root manager available for memory write");
}

Result<const char*> get_skroot_version() {
    return Result<const char*>::success("1.0.0 (Root Manager Integration)");
}

void cleanup_skroot() {
    if (root_handle != nullptr) {
        dlclose(root_handle);
        root_handle = nullptr;
        root_init = nullptr;
        root_call_kernel = nullptr;
        root_read_mem = nullptr;
        root_write_mem = nullptr;
    }
    initialized = false;
}

} // namespace skroot
} // namespace ukc

