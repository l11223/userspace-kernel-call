#ifndef USERSPACE_KERNEL_CALL_PROCESS_MANAGER_H
#define USERSPACE_KERNEL_CALL_PROCESS_MANAGER_H

#include "data_models.h"
#include "result.h"
#include <vector>
#include <sys/types.h>

namespace ukc {

/**
 * 进程管理器
 * 管理目标进程信息
 */
class ProcessManager {
public:
    ProcessManager();
    ~ProcessManager();
    
    /**
     * 查找进程
     */
    Result<pid_t> findProcessByName(const std::string& processName);
    
    /**
     * 验证进程是否存在
     */
    bool isProcessAlive(pid_t pid) const;
    
    /**
     * 获取进程内存映射
     */
    Result<std::vector<MemoryRegion>> getMemoryMaps(pid_t pid);
    
    /**
     * 验证地址是否在有效范围内
     */
    bool isValidAddress(pid_t pid, uintptr_t address);

private:
    /**
     * 解析 /proc/pid/maps 文件
     */
    Result<std::vector<MemoryRegion>> parseMemoryMaps(const std::string& mapsContent);
};

} // namespace ukc

#endif // USERSPACE_KERNEL_CALL_PROCESS_MANAGER_H
