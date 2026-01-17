#ifndef USERSPACE_KERNEL_CALL_DATA_MODELS_H
#define USERSPACE_KERNEL_CALL_DATA_MODELS_H

#include <cstdint>
#include <vector>
#include <string>
#include <chrono>

namespace ukc {

/**
 * 特征码模式
 * 用于在内核内存中搜索特定函数的字节序列
 */
struct SignaturePattern {
    std::vector<uint8_t> bytes;        // 特征字节序列
    std::vector<bool> mask;            // 掩码，true 表示该字节必须匹配
    size_t alignment = 4;              // 对齐要求（ARM64 通常是 4 字节）
    
    /**
     * 从十六进制字符串创建模式
     * 支持通配符 "??" 表示任意字节
     * 
     * 示例：
     *   SignaturePattern::fromHexString(
     *       "1F 20 03 D5 ?? ?? ?? ?? C0 03 5F D6",
     *       "FF FF FF FF 00 00 00 00 FF FF FF FF"
     *   )
     */
    static SignaturePattern fromHexString(
        const std::string& hexString,
        const std::string& maskString = ""
    );
    
    /**
     * 检查模式是否有效
     */
    bool isValid() const;
    
    /**
     * 获取模式大小（字节数）
     */
    size_t size() const {
        return bytes.size();
    }
};

/**
 * 内核函数信息
 */
struct KernelFunctionInfo {
    std::string name;                  // 函数名称
    uintptr_t address = 0;             // 函数地址
    SignaturePattern pattern;          // 特征码模式
    bool isLocated = false;            // 是否已定位
    std::chrono::time_point<std::chrono::steady_clock> locatedTime;  // 定位时间
};

/**
 * 内存操作类型
 */
enum class OperationType {
    Read,
    Write
};

/**
 * 单个内存操作
 */
struct MemoryOperation {
    OperationType type;
    uintptr_t address = 0;
    std::vector<uint8_t> data;         // Write 时使用
    size_t size = 0;                   // Read 时使用
    
    // 操作结果
    bool success = false;
    std::vector<uint8_t> result;       // Read 时的结果
    std::string errorMessage;
};

/**
 * 内核调用上下文
 */
struct KernelCallContext {
    uintptr_t functionAddress = 0;
    std::vector<uint64_t> arguments;
    uint64_t returnValue = 0;
    bool hasException = false;
    std::string exceptionMessage;
    
    // 性能统计
    std::chrono::microseconds executionTime{0};
};

/**
 * 内存区域信息
 */
struct MemoryRegion {
    uintptr_t start = 0;
    uintptr_t end = 0;
    std::string permissions;           // "rwxp" 格式
    std::string path;                  // 映射文件路径
    
    /**
     * 获取区域大小
     */
    size_t size() const {
        return end - start;
    }
    
    /**
     * 检查是否可读
     */
    bool isReadable() const {
        return permissions.length() > 0 && permissions[0] == 'r';
    }
    
    /**
     * 检查是否可写
     */
    bool isWritable() const {
        return permissions.length() > 1 && permissions[1] == 'w';
    }
    
    /**
     * 检查是否可执行
     */
    bool isExecutable() const {
        return permissions.length() > 2 && permissions[2] == 'x';
    }
    
    /**
     * 检查是否是私有映射
     */
    bool isPrivate() const {
        return permissions.length() > 3 && permissions[3] == 'p';
    }
};

} // namespace ukc

#endif // USERSPACE_KERNEL_CALL_DATA_MODELS_H
