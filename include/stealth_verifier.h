#ifndef USERSPACE_KERNEL_CALL_STEALTH_VERIFIER_H
#define USERSPACE_KERNEL_CALL_STEALTH_VERIFIER_H

#include "result.h"
#include <vector>
#include <string>
#include <set>

namespace ukc {

/**
 * 系统状态快照
 * 用于检测操作前后的系统变化
 */
struct SystemSnapshot {
    std::set<std::string> loadedModules;      // 已加载的内核模块
    std::set<std::string> devFiles;           // /dev 中的文件
    std::set<std::string> sysModuleFiles;     // /sys/module 中的文件
    size_t openFileDescriptors;               // 打开的文件描述符数量
    size_t allocatedMemory;                   // 分配的内存大小
};

/**
 * 无痕操作验证器
 * 验证操作不留下检测痕迹
 */
class StealthVerifier {
public:
    StealthVerifier();
    ~StealthVerifier();
    
    /**
     * 获取当前系统状态快照
     */
    Result<SystemSnapshot> captureSystemState();
    
    /**
     * 检查是否有新的内核模块被加载
     */
    Result<bool> hasNewModulesLoaded(
        const SystemSnapshot& before,
        const SystemSnapshot& after
    );
    
    /**
     * 检查是否有新的持久化文件被创建
     */
    Result<bool> hasNewPersistentFiles(
        const SystemSnapshot& before,
        const SystemSnapshot& after
    );
    
    /**
     * 检查资源是否被正确清理
     */
    Result<bool> areResourcesCleaned(
        const SystemSnapshot& before,
        const SystemSnapshot& after
    );
    
    /**
     * 验证完整的无痕操作
     */
    Result<bool> verifyStealthOperation(
        const SystemSnapshot& before,
        const SystemSnapshot& after
    );

private:
    /**
     * 读取已加载的内核模块列表
     */
    Result<std::set<std::string>> readLoadedModules();
    
    /**
     * 读取 /dev 中的文件列表
     */
    Result<std::set<std::string>> readDevFiles();
    
    /**
     * 读取 /sys/module 中的文件列表
     */
    Result<std::set<std::string>> readSysModuleFiles();
    
    /**
     * 获取当前进程的打开文件描述符数量
     */
    Result<size_t> getOpenFileDescriptorCount();
    
    /**
     * 获取当前进程的内存使用量
     */
    Result<size_t> getProcessMemoryUsage();
};

} // namespace ukc

#endif // USERSPACE_KERNEL_CALL_STEALTH_VERIFIER_H
