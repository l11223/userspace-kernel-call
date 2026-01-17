#include <gtest/gtest.h>
#include <rapidcheck.h>
#include "kernel_function_locator.h"

using namespace ukc;

/**
 * Property 2: 地址验证
 * 
 * For any 定位到的内核函数地址或用户提供的地址，系统应该验证该地址在有效的
 * 内存范围内（内核地址在内核空间，进程地址在进程空间），无效地址应该被拒绝。
 * 
 * Validates: Requirements 1.3, 3.2, 6.1
 */
class KernelFunctionLocatorPropertyTest : public ::testing::Test {
protected:
    KernelFunctionLocator locator;
    
    void SetUp() override {
        auto result = locator.initialize();
        EXPECT_TRUE(result.isSuccess());
    }
};

// 属性测试：地址验证
TEST_F(KernelFunctionLocatorPropertyTest, PropertyAddressValidation) {
    rc::check("All validated addresses must be within kernel range",
        [this](uint64_t testAddress) {
            // 如果地址被接受，它必须在有效范围内
            if (locator.isValidKernelAddress(testAddress)) {
                RC_ASSERT(testAddress >= locator.getKernelBaseAddress());
                RC_ASSERT(testAddress < locator.getKernelBaseAddress() + 
                         locator.getKernelSize());
            }
        }
    );
}

/**
 * Property 10: 地址缓存
 * 
 * For any 内核函数，第一次定位后，后续的查询应该使用缓存的地址而不是重新搜索。
 * 
 * Validates: Requirements 8.1
 */
TEST_F(KernelFunctionLocatorPropertyTest, PropertyAddressCache) {
    rc::check("Cached addresses should be returned on subsequent queries",
        [this](const std::string& funcName, uint64_t addr) {
            // 跳过空函数名
            if (funcName.empty()) {
                return;
            }
            
            // 缓存地址
            locator.cacheAddress(funcName, addr);
            
            // 获取缓存的地址
            auto cached = locator.getCachedAddress(funcName);
            RC_ASSERT(cached.has_value());
            RC_ASSERT(cached.value() == addr);
            
            // 再次获取应该返回相同的值
            auto cached2 = locator.getCachedAddress(funcName);
            RC_ASSERT(cached2.has_value());
            RC_ASSERT(cached2.value() == addr);
        }
    );
}

// 属性测试：缓存一致性
TEST_F(KernelFunctionLocatorPropertyTest, PropertyCacheConsistency) {
    rc::check("Cache should maintain consistency across multiple operations",
        [this](const std::vector<std::pair<std::string, uint64_t>>& entries) {
            // 缓存所有条目
            for (const auto& entry : entries) {
                if (!entry.first.empty()) {
                    locator.cacheAddress(entry.first, entry.second);
                }
            }
            
            // 验证所有条目都可以检索
            for (const auto& entry : entries) {
                if (!entry.first.empty()) {
                    auto cached = locator.getCachedAddress(entry.first);
                    RC_ASSERT(cached.has_value());
                    RC_ASSERT(cached.value() == entry.second);
                }
            }
        }
    );
}

// 属性测试：内核地址范围
TEST_F(KernelFunctionLocatorPropertyTest, PropertyKernelAddressRange) {
    rc::check("Kernel base address and size should be valid",
        [this]() {
            uintptr_t baseAddr = locator.getKernelBaseAddress();
            size_t kernelSize = locator.getKernelSize();
            
            // 基址不应该为 0
            RC_ASSERT(baseAddr != 0);
            
            // 大小不应该为 0
            RC_ASSERT(kernelSize != 0);
            
            // 基址应该在合理的范围内（ARM64 内核空间）
            RC_ASSERT(baseAddr >= 0xFFFFFF8000000000UL);
            
            // 大小应该在合理的范围内
            RC_ASSERT(kernelSize <= 0x1000000000UL);  // 最多 64GB
        }
    );
}

// 属性测试：缓存不存在的函数
TEST_F(KernelFunctionLocatorPropertyTest, PropertyNonexistentFunctionCache) {
    rc::check("Non-existent functions should not be in cache",
        [this](const std::string& funcName) {
            // 跳过空函数名
            if (funcName.empty()) {
                return;
            }
            
            // 不缓存任何东西
            auto cached = locator.getCachedAddress(funcName);
            
            // 应该没有缓存
            RC_ASSERT(!cached.has_value());
        }
    );
}

// 属性测试：地址验证边界
TEST_F(KernelFunctionLocatorPropertyTest, PropertyAddressValidationBoundary) {
    rc::check("Address validation should respect boundaries",
        [this]() {
            uintptr_t baseAddr = locator.getKernelBaseAddress();
            size_t kernelSize = locator.getKernelSize();
            
            // 基址应该有效
            RC_ASSERT(locator.isValidKernelAddress(baseAddr));
            
            // 基址 + 大小 - 1 应该有效
            RC_ASSERT(locator.isValidKernelAddress(baseAddr + kernelSize - 1));
            
            // 基址 - 1 应该无效
            RC_ASSERT(!locator.isValidKernelAddress(baseAddr - 1));
            
            // 基址 + 大小 应该无效
            RC_ASSERT(!locator.isValidKernelAddress(baseAddr + kernelSize));
        }
    );
}
