#include "skroot_interface.h"
#include <dlfcn.h>
#include <cstring>
#include <unistd.h>

namespace ukc {
namespace skroot {

// SKRoot 库句柄
static void* skroot_handle = nullptr;

// SKRoot 函数指针
typedef int (*skroot_init_t)();
typedef int (*skroot_call_kernel_t)(uintptr_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
typedef int (*skroot_read_mem_t)(uintptr_t, void*, size_t);
typedef int (*skroot_write_mem_t)(uintptr_t, const void*, size_t);
typedef const char* (*skroot_version_t)();

static skroot_init_t skroot_init = nullptr;
static skroot_call_kernel_t skroot_call_kernel = nullptr;
static skroot_read_mem_t skroot_read_mem = nullptr;
static skroot_write_mem_t skroot_write_mem = nullptr;
static skroot_version_t skroot_version = nullptr;

/**
 * 加载 SKRoot 库
 */
static Result<bool> load_skroot_library() {
    if (skroot_handle != nullptr) {
        return Result<bool>::success(true);
    }

    // 尝试加载 SKRoot 库
    const char* skroot_paths[] = {
        "/system/lib64/libskroot.so",
        "/system/lib/libskroot.so",
        "/data/local/tmp/libskroot.so",
        "libskroot.so",
        nullptr
    };

    for (int i = 0; skroot_paths[i] != nullptr; i++) {
        skroot_handle = dlopen(skroot_paths[i], RTLD_LAZY);
        if (skroot_handle != nullptr) {
            break;
        }
    }

    if (skroot_handle == nullptr) {
        return Result<bool>::error("Failed to load SKRoot library: " + std::string(dlerror()));
    }

    // 加载函数指针
    skroot_init = (skroot_init_t)dlsym(skroot_handle, "skroot_init");
    skroot_call_kernel = (skroot_call_kernel_t)dlsym(skroot_handle, "skroot_call_kernel");
    skroot_read_mem = (skroot_read_mem_t)dlsym(skroot_handle, "skroot_read_mem");
    skroot_write_mem = (skroot_write_mem_t)dlsym(skroot_handle, "skroot_write_mem");
    skroot_version = (skroot_version_t)dlsym(skroot_handle, "skroot_version");

    if (skroot_init == nullptr || skroot_call_kernel == nullptr) {
        dlclose(skroot_handle);
        skroot_handle = nullptr;
        return Result<bool>::error("Failed to load SKRoot functions");
    }

    return Result<bool>::success(true);
}

Result<bool> initialize_skroot() {
    auto load_result = load_skroot_library();
    if (!load_result.isSuccess()) {
        return load_result;
    }

    // 初始化 SKRoot
    int ret = skroot_init();
    if (ret != 0) {
        return Result<bool>::error("SKRoot initialization failed with code: " + std::to_string(ret));
    }

    return Result<bool>::success(true);
}

Result<bool> is_skroot_available() {
    auto load_result = load_skroot_library();
    if (!load_result.isSuccess()) {
        return Result<bool>::success(false);
    }

    // 检查是否可以调用 SKRoot
    int ret = skroot_init();
    return Result<bool>::success(ret == 0);
}

Result<uint64_t> call_kernel_function(
    uintptr_t kernel_func_addr,
    const uint64_t* args,
    size_t arg_count
) {
    if (skroot_call_kernel == nullptr) {
        return Result<uint64_t>::error("SKRoot not initialized");
    }

    if (arg_count > 6) {
        return Result<uint64_t>::error("Too many arguments (max 6)");
    }

    // 准备参数
    uint64_t arg0 = (arg_count > 0) ? args[0] : 0;
    uint64_t arg1 = (arg_count > 1) ? args[1] : 0;
    uint64_t arg2 = (arg_count > 2) ? args[2] : 0;
    uint64_t arg3 = (arg_count > 3) ? args[3] : 0;
    uint64_t arg4 = (arg_count > 4) ? args[4] : 0;
    uint64_t arg5 = (arg_count > 5) ? args[5] : 0;

    // 调用内核函数
    int ret = skroot_call_kernel(kernel_func_addr, arg0, arg1, arg2, arg3, arg4, arg5);
    
    if (ret < 0) {
        return Result<uint64_t>::error("SKRoot kernel call failed with code: " + std::to_string(ret));
    }

    return Result<uint64_t>::success((uint64_t)ret);
}

Result<size_t> read_kernel_memory(
    uintptr_t kernel_addr,
    uint8_t* buffer,
    size_t size
) {
    if (skroot_read_mem == nullptr) {
        return Result<size_t>::error("SKRoot not initialized");
    }

    if (buffer == nullptr || size == 0) {
        return Result<size_t>::error("Invalid buffer or size");
    }

    int ret = skroot_read_mem(kernel_addr, buffer, size);
    
    if (ret < 0) {
        return Result<size_t>::error("SKRoot read memory failed with code: " + std::to_string(ret));
    }

    return Result<size_t>::success((size_t)ret);
}

Result<size_t> write_kernel_memory(
    uintptr_t kernel_addr,
    const uint8_t* data,
    size_t size
) {
    if (skroot_write_mem == nullptr) {
        return Result<size_t>::error("SKRoot not initialized");
    }

    if (data == nullptr || size == 0) {
        return Result<size_t>::error("Invalid data or size");
    }

    int ret = skroot_write_mem(kernel_addr, data, size);
    
    if (ret < 0) {
        return Result<size_t>::error("SKRoot write memory failed with code: " + std::to_string(ret));
    }

    return Result<size_t>::success((size_t)ret);
}

Result<const char*> get_skroot_version() {
    if (skroot_version == nullptr) {
        return Result<const char*>::error("SKRoot not initialized");
    }

    const char* version = skroot_version();
    if (version == nullptr) {
        return Result<const char*>::error("Failed to get SKRoot version");
    }

    return Result<const char*>::success(version);
}

void cleanup_skroot() {
    if (skroot_handle != nullptr) {
        dlclose(skroot_handle);
        skroot_handle = nullptr;
        skroot_init = nullptr;
        skroot_call_kernel = nullptr;
        skroot_read_mem = nullptr;
        skroot_write_mem = nullptr;
        skroot_version = nullptr;
    }
}

} // namespace skroot
} // namespace ukc
