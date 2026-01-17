#ifndef USERSPACE_KERNEL_CALL_KERNEL_FUNCTION_LOCATOR_H
#define USERSPACE_KERNEL_CALL_KERNEL_FUNCTION_LOCATOR_H

#include "data_models.h"
#include "result.h"
#include <map>
#include <memory>

namespace ukc {

/**
 * 内核函数定位器
 * 负责通过特征码搜索定位内核函数地址
 */
class KernelFunctionLocator {
public:
    KernelFunctionLocator();
    ~KernelFunctionLocator();
    
    /**
     * 初始化定位器，加载内核内存映射
     */
    Result<void> initialize();
    
    /**
     * 通过特征码搜索定位函数
     */
    Result<uintptr_t> locateFunction(
        const std::string& functionName,
        const SignaturePattern& pattern
    );
    
    /**
     * 验证地址是否在有效的内核内存范围内
     */
    bool isValidKernelAddress(uintptr_t address) const;
    
    /**
     * 缓存已定位的函数地址
     */
    void cacheAddress(const std::string& functionName, uintptr_t address);
    
    /**
     * 获取缓存的地址
     */
    std::optional<uintptr_t> getCachedAddress(const std::string& functionName) const;
    
    /**
     * 获取内核基址
     */
    uintptr_t getKernelBaseAddress() const {
        return kernelBaseAddress_;
    }
    
    /**
     * 获取内核大小
     */
    size_t getKernelSize() const {
        return kernelSize_;
    }

private:
    std::map<std::string, uintptr_t> addressCache_;
    uintptr_t kernelBaseAddress_ = 0;
    size_t kernelSize_ = 0;
    bool initialized_ = false;
    
    /**
     * 加载内核内存映射
     */
    Result<void> loadKernelMemoryMap();
    
    /**
     * 通过 Magisk 接口定位函数（安卓15推荐）
     */
    Result<uintptr_t> locateFunctionViaMagisk(const std::string& functionName);
    
    /**
     * 从 /proc/kallsyms 定位函数
     */
    Result<uintptr_t> locateFunctionFromKallsyms(const std::string& functionName);
};

} // namespace ukc

#endif // USERSPACE_KERNEL_CALL_KERNEL_FUNCTION_LOCATOR_H
