#include <gtest/gtest.h>
#include "signature_scanner.h"

using namespace ukc;

class SignatureScannerTest : public ::testing::Test {
protected:
    // 测试缓冲区
    uint8_t buffer[256];
    
    void SetUp() override {
        // 初始化测试缓冲区
        for (int i = 0; i < 256; ++i) {
            buffer[i] = i & 0xFF;
        }
    }
};

// 测试基本的特征码搜索
TEST_F(SignatureScannerTest, ScanBasic) {
    auto pattern = SignaturePattern::fromHexString("01 02 03 04");
    auto result = SignatureScanner::scan(buffer, 256, pattern);
    
    EXPECT_TRUE(result.isSuccess());
    const auto& matches = result.value();
    EXPECT_EQ(matches.size(), 1);
    EXPECT_EQ(matches[0], 1);  // 模式从偏移 1 开始
}

// 测试通配符搜索
TEST_F(SignatureScannerTest, ScanWithWildcard) {
    auto pattern = SignaturePattern::fromHexString("01 ?? 03 04");
    auto result = SignatureScanner::scan(buffer, 256, pattern);
    
    EXPECT_TRUE(result.isSuccess());
    const auto& matches = result.value();
    EXPECT_EQ(matches.size(), 1);
    EXPECT_EQ(matches[0], 1);
}

// 测试未找到的模式
TEST_F(SignatureScannerTest, ScanNotFound) {
    auto pattern = SignaturePattern::fromHexString("FF FF FF FF");
    auto result = SignatureScanner::scan(buffer, 256, pattern);
    
    EXPECT_TRUE(result.isSuccess());
    const auto& matches = result.value();
    EXPECT_EQ(matches.size(), 0);
}

// 测试空缓冲区
TEST_F(SignatureScannerTest, ScanNullBuffer) {
    auto pattern = SignaturePattern::fromHexString("01 02 03 04");
    auto result = SignatureScanner::scan(nullptr, 256, pattern);
    
    EXPECT_TRUE(result.isError());
}

// 测试无效的模式
TEST_F(SignatureScannerTest, ScanInvalidPattern) {
    auto pattern = SignaturePattern::fromHexString("");
    auto result = SignatureScanner::scan(buffer, 256, pattern);
    
    EXPECT_TRUE(result.isError());
}

// 测试模式大于缓冲区
TEST_F(SignatureScannerTest, ScanPatternLargerThanBuffer) {
    auto pattern = SignaturePattern::fromHexString("01 02 03 04");
    auto result = SignatureScanner::scan(buffer, 2, pattern);
    
    EXPECT_TRUE(result.isError());
}

// 测试 scanFirst
TEST_F(SignatureScannerTest, ScanFirstBasic) {
    auto pattern = SignaturePattern::fromHexString("01 02 03 04");
    auto result = SignatureScanner::scanFirst(buffer, 256, pattern);
    
    EXPECT_TRUE(result.isSuccess());
    EXPECT_EQ(result.value(), 1);
}

// 测试 scanFirst 未找到
TEST_F(SignatureScannerTest, ScanFirstNotFound) {
    auto pattern = SignaturePattern::fromHexString("FF FF FF FF");
    auto result = SignatureScanner::scanFirst(buffer, 256, pattern);
    
    EXPECT_TRUE(result.isError());
}
