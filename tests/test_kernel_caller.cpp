#include <gtest/gtest.h>
#include "kernel_caller.h"

using namespace ukc;

class KernelCallerTest : public ::testing::Test {
protected:
    KernelCaller caller;
};

// 测试 Root 权限检查
TEST_F(KernelCallerTest, RootPrivilegeCheck) {
    auto result = caller.initialize();
    
    // 如果不是 root 运行，应该失败
    if (geteuid() != 0) {
        EXPECT_TRUE(result.isError());
        EXPECT_FALSE(caller.hasRootPrivilege());
    } else {
        // 如果是 root，应该成功
        EXPECT_TRUE(result.isSuccess());
        EXPECT_TRUE(caller.hasRootPrivilege());
    }
}

// 测试未初始化时调用内核函数
TEST_F(KernelCallerTest, CallWithoutInitialize) {
    std::vector<uint64_t> args;
    auto result = caller.callKernelFunction(0xFFFFFF8000001000UL, args);
    
    EXPECT_TRUE(result.isError());
}

// 测试参数数量限制
TEST_F(KernelCallerTest, TooManyArguments) {
    auto initResult = caller.initialize();
    
    if (initResult.isSuccess()) {
        std::vector<uint64_t> args(7);  // 超过 6 个参数
        auto result = caller.callKernelFunction(0xFFFFFF8000001000UL, args);
        
        EXPECT_TRUE(result.isError());
    }
}
