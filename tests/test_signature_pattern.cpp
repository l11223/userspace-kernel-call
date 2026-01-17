#include <gtest/gtest.h>
#include "data_models.h"

using namespace ukc;

class SignaturePatternTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 测试前的设置
    }
    
    void TearDown() override {
        // 测试后的清理
    }
};

// 测试十六进制字符串解析
TEST_F(SignaturePatternTest, FromHexStringBasic) {
    auto pattern = SignaturePattern::fromHexString("1F 20 03 D5");
    
    EXPECT_EQ(pattern.bytes.size(), 4);
    EXPECT_EQ(pattern.mask.size(), 4);
    EXPECT_EQ(pattern.bytes[0], 0x1F);
    EXPECT_EQ(pattern.bytes[1], 0x20);
    EXPECT_EQ(pattern.bytes[2], 0x03);
    EXPECT_EQ(pattern.bytes[3], 0xD5);
    
    // 所有字节都应该匹配
    for (bool m : pattern.mask) {
        EXPECT_TRUE(m);
    }
}

// 测试通配符处理
TEST_F(SignaturePatternTest, FromHexStringWithWildcard) {
    auto pattern = SignaturePattern::fromHexString("1F 20 ?? D5");
    
    EXPECT_EQ(pattern.bytes.size(), 4);
    EXPECT_EQ(pattern.mask.size(), 4);
    EXPECT_EQ(pattern.bytes[0], 0x1F);
    EXPECT_EQ(pattern.bytes[1], 0x20);
    EXPECT_EQ(pattern.bytes[2], 0x00);  // 通配符
    EXPECT_EQ(pattern.bytes[3], 0xD5);
    
    EXPECT_TRUE(pattern.mask[0]);
    EXPECT_TRUE(pattern.mask[1]);
    EXPECT_FALSE(pattern.mask[2]);  // 通配符不匹配
    EXPECT_TRUE(pattern.mask[3]);
}

// 测试掩码字符串
TEST_F(SignaturePatternTest, FromHexStringWithMask) {
    auto pattern = SignaturePattern::fromHexString(
        "1F 20 03 D5",
        "FF FF 00 FF"
    );
    
    EXPECT_EQ(pattern.bytes.size(), 4);
    EXPECT_EQ(pattern.mask.size(), 4);
    
    EXPECT_TRUE(pattern.mask[0]);
    EXPECT_TRUE(pattern.mask[1]);
    EXPECT_FALSE(pattern.mask[2]);  // 掩码为 00
    EXPECT_TRUE(pattern.mask[3]);
}

// 测试无效的十六进制字符串
TEST_F(SignaturePatternTest, FromHexStringInvalid) {
    auto pattern = SignaturePattern::fromHexString("1F 20 ZZ D5");
    
    EXPECT_TRUE(pattern.bytes.empty());
    EXPECT_TRUE(pattern.mask.empty());
}

// 测试空模式
TEST_F(SignaturePatternTest, FromHexStringEmpty) {
    auto pattern = SignaturePattern::fromHexString("");
    
    EXPECT_TRUE(pattern.bytes.empty());
    EXPECT_TRUE(pattern.mask.empty());
}

// 测试模式有效性检查
TEST_F(SignaturePatternTest, IsValidPattern) {
    auto validPattern = SignaturePattern::fromHexString("1F 20 03 D5");
    EXPECT_TRUE(validPattern.isValid());
    
    auto emptyPattern = SignaturePattern::fromHexString("");
    EXPECT_FALSE(emptyPattern.isValid());
    
    // 所有字节都是通配符的模式应该无效
    auto allWildcard = SignaturePattern::fromHexString("?? ?? ?? ??");
    EXPECT_FALSE(allWildcard.isValid());
}

// 测试模式大小
TEST_F(SignaturePatternTest, PatternSize) {
    auto pattern = SignaturePattern::fromHexString("1F 20 03 D5 C0 03 5F D6");
    EXPECT_EQ(pattern.size(), 8);
}
