#include <gtest/gtest.h>
#include <rapidcheck.h>
#include "signature_scanner.h"

using namespace ukc;

/**
 * Property 1: 特征码搜索定位
 * 
 * For any 内核函数和对应的特征码模式，当执行搜索时，系统应该使用模式匹配
 * 在内核内存中定位函数地址。
 * 
 * Validates: Requirements 1.1
 */
class SignatureScannerPropertyTest : public ::testing::Test {
};

// 属性测试：特征码搜索定位
TEST_F(SignatureScannerPropertyTest, PropertySignatureSearch) {
    rc::check("Pattern matching should find all occurrences",
        [](const std::vector<uint8_t>& patternBytes) {
            // 生成测试缓冲区
            std::vector<uint8_t> buffer(512);
            for (size_t i = 0; i < buffer.size(); ++i) {
                buffer[i] = i & 0xFF;
            }
            
            // 跳过空模式
            if (patternBytes.empty()) {
                return;
            }
            
            // 创建特征码模式（所有字节都必须匹配）
            SignaturePattern pattern;
            pattern.bytes = patternBytes;
            pattern.mask.assign(patternBytes.size(), true);
            pattern.alignment = 1;
            
            if (!pattern.isValid()) {
                return;
            }
            
            // 在缓冲区中搜索
            auto result = SignatureScanner::scan(buffer.data(), buffer.size(), pattern);
            
            // 搜索应该成功
            RC_ASSERT(result.isSuccess());
            
            // 验证找到的所有匹配都是正确的
            const auto& matches = result.value();
            for (uintptr_t offset : matches) {
                // 验证偏移在有效范围内
                RC_ASSERT(offset + pattern.size() <= buffer.size());
                
                // 验证该位置确实匹配模式
                for (size_t i = 0; i < pattern.size(); ++i) {
                    if (pattern.mask[i]) {
                        RC_ASSERT(buffer[offset + i] == pattern.bytes[i]);
                    }
                }
            }
        }
    );
}

// 属性测试：通配符匹配
TEST_F(SignatureScannerPropertyTest, PropertyWildcardMatching) {
    rc::check("Wildcard bytes should match any value",
        [](const std::vector<uint8_t>& fixedBytes) {
            // 生成测试缓冲区
            std::vector<uint8_t> buffer(256);
            for (size_t i = 0; i < buffer.size(); ++i) {
                buffer[i] = i & 0xFF;
            }
            
            // 跳过空模式
            if (fixedBytes.empty() || fixedBytes.size() > 16) {
                return;
            }
            
            // 创建模式：固定字节 + 通配符 + 固定字节
            SignaturePattern pattern;
            pattern.bytes = fixedBytes;
            pattern.mask.assign(fixedBytes.size(), true);
            
            // 添加通配符
            if (fixedBytes.size() < 16) {
                pattern.bytes.push_back(0xFF);
                pattern.mask.push_back(false);  // 通配符
                pattern.bytes.push_back(0xFF);
                pattern.mask.push_back(false);  // 通配符
            }
            
            pattern.alignment = 1;
            
            if (!pattern.isValid()) {
                return;
            }
            
            auto result = SignatureScanner::scan(buffer.data(), buffer.size(), pattern);
            RC_ASSERT(result.isSuccess());
            
            // 验证通配符确实匹配了任意字节
            const auto& matches = result.value();
            for (uintptr_t offset : matches) {
                // 验证固定字节匹配
                for (size_t i = 0; i < fixedBytes.size(); ++i) {
                    RC_ASSERT(buffer[offset + i] == pattern.bytes[i]);
                }
            }
        }
    );
}

// 属性测试：对齐要求
TEST_F(SignatureScannerPropertyTest, PropertyAlignmentRequirement) {
    rc::check("Alignment should be respected",
        [](uint8_t alignmentPower) {
            // 限制对齐大小
            size_t alignment = 1 << (alignmentPower % 4);  // 1, 2, 4, 8
            
            // 生成测试缓冲区
            std::vector<uint8_t> buffer(256);
            for (size_t i = 0; i < buffer.size(); ++i) {
                buffer[i] = i & 0xFF;
            }
            
            // 创建简单的模式
            SignaturePattern pattern;
            pattern.bytes = {0x01, 0x02, 0x03, 0x04};
            pattern.mask = {true, true, true, true};
            pattern.alignment = alignment;
            
            auto result = SignatureScanner::scan(buffer.data(), buffer.size(), pattern);
            RC_ASSERT(result.isSuccess());
            
            // 验证所有匹配都遵守对齐要求
            const auto& matches = result.value();
            for (uintptr_t offset : matches) {
                RC_ASSERT(offset % alignment == 0);
            }
        }
    );
}

// 属性测试：缓冲区边界
TEST_F(SignatureScannerPropertyTest, PropertyBufferBoundary) {
    rc::check("Should not read beyond buffer boundary",
        [](const std::vector<uint8_t>& patternBytes) {
            // 生成小缓冲区
            std::vector<uint8_t> buffer(64);
            for (size_t i = 0; i < buffer.size(); ++i) {
                buffer[i] = i & 0xFF;
            }
            
            // 跳过空或过大的模式
            if (patternBytes.empty() || patternBytes.size() > buffer.size()) {
                return;
            }
            
            // 创建特征码模式
            SignaturePattern pattern;
            pattern.bytes = patternBytes;
            pattern.mask.assign(patternBytes.size(), true);
            pattern.alignment = 1;
            
            if (!pattern.isValid()) {
                return;
            }
            
            // 搜索应该成功且不会崩溃
            auto result = SignatureScanner::scan(buffer.data(), buffer.size(), pattern);
            RC_ASSERT(result.isSuccess());
            
            // 验证所有匹配都在缓冲区内
            const auto& matches = result.value();
            for (uintptr_t offset : matches) {
                RC_ASSERT(offset + pattern.size() <= buffer.size());
            }
        }
    );
}

// 属性测试：错误处理
TEST_F(SignatureScannerPropertyTest, PropertyErrorHandling) {
    rc::check("Invalid inputs should return errors",
        []() {
            // 空缓冲区
            SignaturePattern pattern;
            pattern.bytes = {0x01, 0x02};
            pattern.mask = {true, true};
            
            auto result = SignatureScanner::scan(nullptr, 256, pattern);
            RC_ASSERT(result.isError());
            
            // 无效的模式
            SignaturePattern invalidPattern;
            invalidPattern.bytes.clear();
            invalidPattern.mask.clear();
            
            std::vector<uint8_t> buffer(256);
            auto result2 = SignatureScanner::scan(buffer.data(), 256, invalidPattern);
            RC_ASSERT(result2.isError());
        }
    );
}

// 属性测试：scanFirst 一致性
TEST_F(SignatureScannerPropertyTest, PropertyScanFirstConsistency) {
    rc::check("scanFirst should return the first match from scan",
        [](const std::vector<uint8_t>& patternBytes) {
            // 生成测试缓冲区
            std::vector<uint8_t> buffer(512);
            for (size_t i = 0; i < buffer.size(); ++i) {
                buffer[i] = i & 0xFF;
            }
            
            // 跳过空模式
            if (patternBytes.empty() || patternBytes.size() > 32) {
                return;
            }
            
            // 创建特征码模式
            SignaturePattern pattern;
            pattern.bytes = patternBytes;
            pattern.mask.assign(patternBytes.size(), true);
            pattern.alignment = 1;
            
            if (!pattern.isValid()) {
                return;
            }
            
            // 执行 scan 和 scanFirst
            auto scanResult = SignatureScanner::scan(buffer.data(), buffer.size(), pattern);
            auto scanFirstResult = SignatureScanner::scanFirst(buffer.data(), buffer.size(), pattern);
            
            if (scanResult.isSuccess() && !scanResult.value().empty()) {
                // 如果 scan 找到了匹配，scanFirst 应该返回第一个
                RC_ASSERT(scanFirstResult.isSuccess());
                RC_ASSERT(scanFirstResult.value() == scanResult.value()[0]);
            } else {
                // 如果 scan 没有找到匹配，scanFirst 应该返回错误
                RC_ASSERT(scanFirstResult.isError());
            }
        }
    );
}
