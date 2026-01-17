#ifndef USERSPACE_KERNEL_CALL_SIGNATURE_SCANNER_H
#define USERSPACE_KERNEL_CALL_SIGNATURE_SCANNER_H

#include "data_models.h"
#include "result.h"
#include <vector>
#include <cstdint>

namespace ukc {

/**
 * 特征码扫描器
 * 在内存缓冲区中搜索特征码模式
 */
class SignatureScanner {
public:
    /**
     * 在内存缓冲区中搜索特征码
     * 
     * @param buffer 内存缓冲区
     * @param bufferSize 缓冲区大小
     * @param pattern 特征码模式
     * @return 找到的地址列表（相对于缓冲区起始地址）
     */
    static Result<std::vector<uintptr_t>> scan(
        const uint8_t* buffer,
        size_t bufferSize,
        const SignaturePattern& pattern
    );
    
    /**
     * 在内存缓冲区中搜索单个特征码
     * 
     * @param buffer 内存缓冲区
     * @param bufferSize 缓冲区大小
     * @param pattern 特征码模式
     * @return 找到的地址（相对于缓冲区起始地址），如果未找到返回 0
     */
    static Result<uintptr_t> scanFirst(
        const uint8_t* buffer,
        size_t bufferSize,
        const SignaturePattern& pattern
    );

private:
    /**
     * 检查缓冲区中的特定位置是否匹配特征码
     */
    static bool matchesPattern(
        const uint8_t* buffer,
        size_t offset,
        const SignaturePattern& pattern
    );
};

} // namespace ukc

#endif // USERSPACE_KERNEL_CALL_SIGNATURE_SCANNER_H
