#include "kernel_function_locator.h"
#include "signature_scanner.h"
#include "magisk_interface.h"
#include <optional>
#include <fstream>
#include <sstream>
#include <cstring>

namespace ukc {

KernelFunctionLocator::KernelFunctionLocator() = default;

KernelFunctionLocator::~KernelFunctionLocator() = default;

Result<void> KernelFunctionLocator::initialize() {
    if (initialized_) {
        return Result<void>::success();
    }
    
    auto mapResult = loadKernelMemoryMap();
    if (mapResult.isError()) {
        return mapResult;
    }
    
    initialized_ = true;
    return Result<void>::success();
}

Result<uintptr_t> KernelFunctionLocator::locateFunction(
    const std::string& functionName,
    const SignaturePattern& pattern
) {
    if (!initialized_) {
        return Result<uintptr_t>::error("KernelFunctionLocator not initialized");
    }
    
    // 检查缓存
    auto cached = getCachedAddress(functionName);
    if (cached.has_value()) {
        return Result<uintptr_t>::success(cached.value());
    }
    
    if (!pattern.isValid()) {
        return Result<uintptr_t>::error("Invalid signature pattern");
    }
    
    // 第1步：尝试通过 Magisk 接口查找（安卓15推荐）
    auto magiskResult = locateFunctionViaMagisk(functionName);
    if (magiskResult.isSuccess()) {
        uintptr_t addr = magiskResult.value();
        cacheAddress(functionName, addr);
        return Result<uintptr_t>::success(addr);
    }
    
    // 第2步：尝试从 /proc/kallsyms 读取（备用方案）
    auto kallsymsResult = locateFunctionFromKallsyms(functionName);
    if (kallsymsResult.isSuccess()) {
        uintptr_t addr = kallsymsResult.value();
        cacheAddress(functionName, addr);
        return Result<uintptr_t>::success(addr);
    }
    
    // 如果 kallsyms 不可用，使用特征码搜索
    // TODO: 实现特征码搜索
    // 这里需要集成 android-kernel-offset-finder 库
    
    return Result<uintptr_t>::error(
        "Function '" + functionName + "' not found (signature search not implemented yet)"
    );
}

bool KernelFunctionLocator::isValidKernelAddress(uintptr_t address) const {
    if (kernelBaseAddress_ == 0 || kernelSize_ == 0) {
        return false;
    }
    
    return address >= kernelBaseAddress_ && 
           address < kernelBaseAddress_ + kernelSize_;
}

void KernelFunctionLocator::cacheAddress(
    const std::string& functionName,
    uintptr_t address
) {
    addressCache_[functionName] = address;
}

std::optional<uintptr_t> KernelFunctionLocator::getCachedAddress(
    const std::string& functionName
) const {
    auto it = addressCache_.find(functionName);
    if (it != addressCache_.end()) {
        return it->second;
    }
    return std::nullopt;
}

Result<void> KernelFunctionLocator::loadKernelMemoryMap() {
    // 尝试从 /proc/kallsyms 读取内核地址范围
    std::ifstream kallsyms("/proc/kallsyms");
    if (kallsyms.is_open()) {
        std::string line;
        uintptr_t minAddr = UINTPTR_MAX;
        uintptr_t maxAddr = 0;
        
        while (std::getline(kallsyms, line)) {
            if (line.empty()) continue;
            
            // 解析 kallsyms 格式: address type name
            std::istringstream iss(line);
            std::string addrStr;
            if (iss >> addrStr) {
                try {
                    uintptr_t addr = std::stoull(addrStr, nullptr, 16);
                    if (addr > 0) {
                        minAddr = std::min(minAddr, addr);
                        maxAddr = std::max(maxAddr, addr);
                    }
                } catch (...) {
                    // 忽略解析错误
                }
            }
        }
        
        if (minAddr != UINTPTR_MAX && maxAddr > minAddr) {
            kernelBaseAddress_ = minAddr;
            kernelSize_ = maxAddr - minAddr + 0x1000;  // 添加一些缓冲
            return Result<void>::success();
        }
    }
    
    // 如果 kallsyms 不可用，使用默认的 ARM64 内核地址范围
    kernelBaseAddress_ = 0xFFFFFF8000000000UL;  // ARM64 内核空间起始地址
    kernelSize_ = 0x100000000UL;                // 假设 4GB 内核空间
    
    return Result<void>::success();
}

Result<uintptr_t> KernelFunctionLocator::locateFunctionFromKallsyms(
    const std::string& functionName
) {
    std::ifstream kallsyms("/proc/kallsyms");
    if (!kallsyms.is_open()) {
        return Result<uintptr_t>::error("/proc/kallsyms not available");
    }
    
    std::string line;
    while (std::getline(kallsyms, line)) {
        if (line.empty()) continue;
        
        // 解析 kallsyms 格式: address type name
        std::istringstream iss(line);
        std::string addrStr, typeStr, nameStr;
        
        if (!(iss >> addrStr >> typeStr >> nameStr)) {
            continue;
        }
        
        // 检查函数名是否匹配
        if (nameStr == functionName) {
            try {
                uintptr_t addr = std::stoull(addrStr, nullptr, 16);
                if (isValidKernelAddress(addr)) {
                    return Result<uintptr_t>::success(addr);
                }
            } catch (...) {
                // 忽略解析错误
            }
        }
    }
    
    return Result<uintptr_t>::error(
        "Function '" + functionName + "' not found in /proc/kallsyms"
    );
}lt<uintptr_t> KernelFunctionLocator::locateFunctionViaMagisk(
    const std::string& functionName
) {
    // 检查 Magisk 是否可用
    if (!magisk::is_magisk_available()) {
        return Result<uintptr_t>::error("Magisk not available");
    }
    
    // 通过 Magisk 接口查找符号
    uintptr_t addr = magisk::magisk_kallsyms_lookup_name(functionName.c_str());
    if (addr == 0) {
        return Result<uintptr_t>::error(
            "Function '" + functionName + "' not found via Magisk"
        );
    }
    
    if (!isValidKernelAddress(addr)) {
        return Result<uintptr_t>::error(
            "Invalid kernel address returned by Magisk for '" + functionName + "'"
        );
    }
    
    return Result<uintptr_t>::success(addr);
}

Result<uintptr_t> KernelFunctionLocator::locateFunctionFromKallsyms(
    const std::string& functionName
) {
    std::ifstream kallsyms("/proc/kallsyms");
    if (!kallsyms.is_open()) {
        return Result<uintptr_t>::error("/proc/kallsyms not available");
    }
    
    std::string line;
    while (std::getline(kallsyms, line)) {
        if (line.empty()) continue;
        
        // 解析 kallsyms 格式: address type name
        std::istringstream iss(line);
        std::string addrStr, typeStr, nameStr;
        
        if (!(iss >> addrStr >> typeStr >> nameStr)) {
            continue;
        }
        
        // 检查函数名是否匹配
        if (nameStr == functionName) {
            try {
                uintptr_t addr = std::stoull(addrStr, nullptr, 16);
                if (isValidKernelAddress(addr)) {
                    return Result<uintptr_t>::success(addr);
                }
            } catch (...) {
                // 忽略解析错误
            }
        }
    }
    
    return Result<uintptr_t>::error(
        "Function '" + functionName + "' not found in /proc/kallsyms"
    );
}

} // namespace ukc
