#include <gtest/gtest.h>
#include "kernel_function_locator.h"

using namespace ukc;

class KernelFunctionLocatorTest : public ::testing::Test {
protected:
    KernelFunctionLocator locator;
    
    void SetUp() override {
        // 初始化定位器
        auto result = locator.initialize();
        EXPECT_TRUE(result.isSuccess());
    }
};

// 测试初始化
TEST_F(KernelFunctionLocatorTest, Initialize) {
    auto result = locator.initialize();
    EXPECT_TRUE(result.isSuccess());
}

// 测试内核地址验证
TEST_F(KernelFunctionLocatorTest, IsValidKernelAddress) {
    // 有效的内核地址
    uintptr_t validAddr = locator.getKernelBaseAddress() + 0x1000;
    EXPECT_TRUE(locator.isValidKernelAddress(validAddr));
    
    // 无效的地址（用户空间）
    uintptr_t invalidAddr = 0x1000;
    EXPECT_FALSE(locator.isValidKernelAddress(invalidAddr));
}

// 测试地址缓存
TEST_F(KernelFunctionLocatorTest, AddressCache) {
    std::string funcName = "test_function";
    uintptr_t addr = 0xFFFFFF8000001000UL;
    
    // 缓存地址
    locator.cacheAddress(funcName, addr);
    
    // 获取缓存的地址
    auto cached = locator.getCachedAddress(funcName);
    EXPECT_TRUE(cached.has_value());
    EXPECT_EQ(cached.value(), addr);
    
    // 获取不存在的地址
    auto notCached = locator.getCachedAddress("nonexistent");
    EXPECT_FALSE(notCached.has_value());
}

// 测试获取内核基址
TEST_F(KernelFunctionLocatorTest, GetKernelBaseAddress) {
    uintptr_t baseAddr = locator.getKernelBaseAddress();
    EXPECT_NE(baseAddr, 0);
}

// 测试获取内核大小
TEST_F(KernelFunctionLocatorTest, GetKernelSize) {
    size_t kernelSize = locator.getKernelSize();
    EXPECT_NE(kernelSize, 0);
}
