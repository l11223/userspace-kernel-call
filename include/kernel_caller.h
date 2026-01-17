#ifndef USERSPACE_KERNEL_CALL_KERNEL_CALLER_H
#define USERSPACE_KERNEL_CALL_KERNEL_CALLER_H

#include "data_models.h"
#include "result.h"
#include <vector>
#include <cstdint>

namespace ukc {

/**
 * 内核调用器
 * 负责在用户态调用内核函数
 */
class KernelCaller {
public:
    KernelCaller();
    ~KernelCaller();
    
    /**
     * 初始化调用器，验证 Root 权限
     */
    Result<void> initialize();
    
    /**
     * 调用内核函数（最多 6 个参数，ARM64 AAPCS）
     */
    Result<uint64_t> callKernelFunction(
        uintptr_t functionAddress,
        const std::vector<uint64_t>& args
    );
    
    /**
     * 准备内核调用环境
     */
    Result<void> prepareKernelContext();
    
    /**
     * 清理内核调用环境
     */
    void cleanupKernelContext();
    
    /**
     * 检查是否有 Root 权限
     */
    bool hasRootPrivilege() const {
        return hasRootPrivilege_;
    }

private:
    bool hasRootPrivilege_ = false;
    bool initialized_ = false;
    
    /**
     * 检查 Root 权限
     */
    Result<void> checkRootPrivilege();
};

} // namespace ukc

#endif // USERSPACE_KERNEL_CALL_KERNEL_CALLER_H
