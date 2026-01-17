#include "data_models.h"
#include <sstream>
#include <cctype>
#include <algorithm>

namespace ukc {

SignaturePattern SignaturePattern::fromHexString(
    const std::string& hexString,
    const std::string& maskString
) {
    SignaturePattern pattern;
    
    // 解析十六进制字符串
    std::istringstream hexStream(hexString);
    std::string hexByte;
    
    while (hexStream >> hexByte) {
        if (hexByte == "??") {
            // 通配符
            pattern.bytes.push_back(0x00);
            pattern.mask.push_back(false);
        } else {
            // 解析十六进制字节
            try {
                uint8_t byte = static_cast<uint8_t>(std::stoi(hexByte, nullptr, 16));
                pattern.bytes.push_back(byte);
                pattern.mask.push_back(true);
            } catch (...) {
                // 无效的十六进制字符串，返回空模式
                pattern.bytes.clear();
                pattern.mask.clear();
                return pattern;
            }
        }
    }
    
    // 如果提供了掩码字符串，使用它
    if (!maskString.empty()) {
        pattern.mask.clear();
        std::istringstream maskStream(maskString);
        std::string maskByte;
        
        while (maskStream >> maskByte) {
            try {
                uint8_t byte = static_cast<uint8_t>(std::stoi(maskByte, nullptr, 16));
                pattern.mask.push_back(byte != 0x00);
            } catch (...) {
                // 无效的掩码字符串
                pattern.bytes.clear();
                pattern.mask.clear();
                return pattern;
            }
        }
    }
    
    // 确保 mask 和 bytes 大小一致
    if (pattern.mask.size() != pattern.bytes.size()) {
        pattern.bytes.clear();
        pattern.mask.clear();
    }
    
    return pattern;
}

bool SignaturePattern::isValid() const {
    // 检查模式是否有效
    if (bytes.empty() || mask.empty()) {
        return false;
    }
    
    if (bytes.size() != mask.size()) {
        return false;
    }
    
    // 至少有一个字节需要匹配
    bool hasMatchByte = false;
    for (bool m : mask) {
        if (m) {
            hasMatchByte = true;
            break;
        }
    }
    
    return hasMatchByte;
}

} // namespace ukc
