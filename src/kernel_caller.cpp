#include "kernel_caller.h"
#include "arm64_assembly_bridge.h"
#include <unistd.h>
#include <cstring>

namespace ukc {

KernelCaller::KernelCaller() = default;

KernelCaller::~KernelCaller() = default;

Result<void> KernelCaller::initialize() {
    if (initialized_) {
        return Result<void>::success();
    }
    
    auto privResult = checkRootPrivilege();
    if (privResult.isError()) {
        return privResult;
    }
    
    auto ctxResult = prepareKernelContext();
    if (ctxResult.isError()) {
        return ctxResult;
    }
    
    initialized_ = true;
    return Result<void>::success();
}

Result<uint64_t> KernelCaller::callKernelFunction(
    uintptr_t functionAddress,
    const std::vector<uint64_t>& args
) {
    if (!initialized_) {
        return Result<uint64_t>::error("KernelCaller not initialized");
    }
    
    if (!hasRootPrivilege_) {
        return Result<uint64_t>::error("Root privilege required");
    }
    
    if (args.size() > 6) {
        return Result<uint64_t>::error("Too many arguments (max 6)");
    }
    
    if (functionAddress == 0) {
        return Result<uint64_t>::error("Invalid kernel function address");
    }
    
    // 准备参数（最多 6 个）
    uint64_t arg0 = args.size() > 0 ? args[0] : 0;
    uint64_t arg1 = args.size() > 1 ? args[1] : 0;
    uint64_t arg2 = args.size() > 2 ? args[2] : 0;
    uint64_t arg3 = args.size() > 3 ? args[3] : 0;
    uint64_t arg4 = args.size() > 4 ? args[4] : 0;
    uint64_t arg5 = args.size() > 5 ? args[5] : 0;
    
    try {
        // 调用内核函数
        uint64_t result = arm64::kernel_call_bridge(
            functionAddress,
            arg0, arg1, arg2, arg3, arg4, arg5
        );
        
        return Result<uint64_t>::success(result);
    } catch (const std::exception& e) {
        return Result<uint64_t>::error(
            std::string("Kernel function call failed: ") + e.what()
        );
    } catch (...) {
        return Result<uint64_t>::error("Kernel function call failed with unknown error");
    }
}

Result<void> KernelCaller::prepareKernelContext() {
    // 准备内核调用环境
    // 这里可以进行必要的初始化，如禁用 SELinux 检查等
    // 目前为空实现
    return Result<void>::success();
}

void KernelCaller::cleanupKernelContext() {
    // 清理内核调用环境
    // 这里可以进行必要的清理工作
    // 目前为空实现
}

Result<void> KernelCaller::checkRootPrivilege() {
    if (geteuid() != 0) {
        return Result<void>::error("Root privilege required (current UID: " + 
                                   std::to_string(geteuid()) + ")");
    }
    
    hasRootPrivilege_ = true;
    return Result<void>::success();
}

} // namespace ukc
