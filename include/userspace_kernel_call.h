#ifndef USERSPACE_KERNEL_CALL_H
#define USERSPACE_KERNEL_CALL_H

#include "result.h"
#include "data_models.h"
#include <vector>
#include <memory>
#include <sys/types.h>

namespace ukc {

// 前向声明
class KernelFunctionLocator;
class KernelCaller;
class ProcessManager;
class MemoryInjector;

/**
 * 用户态调用内核系统
 * 统一的应用层 API
 */
class UserspaceKernelCall {
public:
    UserspaceKernelCall();
    ~UserspaceKernelCall();
    
    /**
     * 初始化系统
     */
    Result<void> initialize();
    
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
     * 查找进程
     */
    Result<pid_t> findProcessByName(const std::string& processName);
    
    /**
     * 获取进程内存映射
     */
    Result<std::vector<MemoryRegion>> getProcessMemoryMaps(pid_t pid);

private:
    std::shared_ptr<KernelFunctionLocator> locator_;
    std::shared_ptr<KernelCaller> caller_;
    std::shared_ptr<ProcessManager> processManager_;
    std::shared_ptr<MemoryInjector> injector_;
    bool initialized_ = false;
};

} // namespace ukc

#endif // USERSPACE_KERNEL_CALL_H
