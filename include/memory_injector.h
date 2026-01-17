#ifndef USERSPACE_KERNEL_CALL_MEMORY_INJECTOR_H
#define USERSPACE_KERNEL_CALL_MEMORY_INJECTOR_H

#include "data_models.h"
#include "result.h"
#include "kernel_function_locator.h"
#include "kernel_caller.h"
#include "process_manager.h"
#include <vector>
#include <memory>
#include <sys/types.h>

namespace ukc {

/**
 * 内存注入器
 * 高层内存注入接口
 */
class MemoryInjector {
public:
    MemoryInjector();
    ~MemoryInjector();
    
    /**
     * 初始化注入器
     */
    Result<void> initialize(
        std::shared_ptr<KernelFunctionLocator> locator,
        std::shared_ptr<KernelCaller> caller,
        std::shared_ptr<ProcessManager> processManager
    );
    
    /**
     * 读取目标进程内存
     */
    Result<std::vector<uint8_t>> readMemory(
        pid_t targetPid,
        uintptr_t address,
        size_t size
    );
    
    /**
     * 写入目标进程内存
     */
    Result<size_t> writeMemory(
        pid_t targetPid,
        uintptr_t address,
        const std::vector<uint8_t>& data
    );
    
    /**
     * 批量内存操作
     */
    Result<void> batchOperations(
        pid_t targetPid,
        std::vector<MemoryOperation>& operations
    );

    /**
     * 读取内核内存（通过 Magisk 接口，安卓15推荐）
     */
    Result<std::vector<uint8_t>> readKernelMemory(
        uintptr_t address,
        size_t size
    );
    
    /**
     * 写入内核内存（通过 Magisk 接口，安卓15推荐）
     */
    Result<size_t> writeKernelMemory(
        uintptr_t address,
        const std::vector<uint8_t>& data
    );

private:
    std::shared_ptr<KernelFunctionLocator> locator_;
    std::shared_ptr<KernelCaller> caller_;
    std::shared_ptr<ProcessManager> processManager_;
    
    // 内核函数地址缓存
    uintptr_t kernelReadMemAddr_ = 0;
    uintptr_t kernelWriteMemAddr_ = 0;
    bool initialized_ = false;
};

} // namespace ukc

#endif // USERSPACE_KERNEL_CALL_MEMORY_INJECTOR_H
