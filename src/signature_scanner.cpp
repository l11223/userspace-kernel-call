#include "signature_scanner.h"
#include <cstring>

namespace ukc {

bool SignatureScanner::matchesPattern(
    const uint8_t* buffer,
    size_t offset,
    const SignaturePattern& pattern
) {
    for (size_t i = 0; i < pattern.bytes.size(); ++i) {
        if (pattern.mask[i]) {
            // 这个字节必须匹配
            if (buffer[offset + i] != pattern.bytes[i]) {
                return false;
            }
        }
        // 如果 mask[i] 为 false，则任意字节都可以
    }
    return true;
}

Result<std::vector<uintptr_t>> SignatureScanner::scan(
    const uint8_t* buffer,
    size_t bufferSize,
    const SignaturePattern& pattern
) {
    // 验证输入
    if (buffer == nullptr) {
        return Result<std::vector<uintptr_t>>::error("Buffer is null");
    }
    
    if (!pattern.isValid()) {
        return Result<std::vector<uintptr_t>>::error("Invalid signature pattern");
    }
    
    if (pattern.size() > bufferSize) {
        return Result<std::vector<uintptr_t>>::error(
            "Pattern size (" + std::to_string(pattern.size()) + 
            ") exceeds buffer size (" + std::to_string(bufferSize) + ")"
        );
    }
    
    std::vector<uintptr_t> results;
    
    // 按对齐要求扫描
    for (size_t offset = 0; offset <= bufferSize - pattern.size(); offset += pattern.alignment) {
        if (matchesPattern(buffer, offset, pattern)) {
            results.push_back(offset);
        }
    }
    
    return Result<std::vector<uintptr_t>>::success(std::move(results));
}

Result<uintptr_t> SignatureScanner::scanFirst(
    const uint8_t* buffer,
    size_t bufferSize,
    const SignaturePattern& pattern
) {
    auto scanResult = scan(buffer, bufferSize, pattern);
    
    if (scanResult.isError()) {
        return Result<uintptr_t>::error(scanResult.errorMessage());
    }
    
    const auto& results = scanResult.value();
    if (results.empty()) {
        return Result<uintptr_t>::error("Pattern not found in buffer");
    }
    
    return Result<uintptr_t>::success(results[0]);
}

} // namespace ukc
