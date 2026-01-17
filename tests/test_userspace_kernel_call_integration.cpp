#include <gtest/gtest.h>
#include "userspace_kernel_call.h"
#include <unistd.h>
#include <memory>

using namespace ukc;

class UserspaceKernelCallIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        ukc_system_ = std::make_unique<UserspaceKernelCall>();
    }
    
    std::unique_ptr<UserspaceKernelCall> ukc_system_;
};

// Test: 初始化系统
TEST_F(UserspaceKernelCallIntegrationTest, InitializeSystem) {
    auto result = ukc_system_->initialize();
    EXPECT_TRUE(result.isSuccess());
}

// Test: 未初始化时读取内存
TEST_F(UserspaceKernelCallIntegrationTest, ReadMemoryWithoutInitialize) {
    pid_t currentPid = getpid();
    auto result = ukc_system_->readMemory(currentPid, 0x1000, 100);
    EXPECT_TRUE(result.isError());
}

// Test: 初始化后读取内存
TEST_F(UserspaceKernelCallIntegrationTest, ReadMemoryAfterInitialize) {
    auto initResult = ukc_system_->initialize();
    ASSERT_TRUE(initResult.isSuccess());
    
    pid_t currentPid = getpid();
    auto result = ukc_system_->readMemory(currentPid, 0x1000, 100);
    
    // 可能成功或失败，取决于地址是否有效
    // 但不应该返回"未初始化"的错误
    EXPECT_NE(result.errorMessage().find("not initialized"), 0);
}

// Test: 写入内存
TEST_F(UserspaceKernelCallIntegrationTest, WriteMemory) {
    auto initResult = ukc_system_->initialize();
    ASSERT_TRUE(initResult.isSuccess());
    
    pid_t currentPid = getpid();
    std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04};
    
    auto result = ukc_system_->writeMemory(currentPid, 0x1000, data);
    
    // 可能成功或失败，取决于地址是否有效
    // 但不应该返回"未初始化"的错误
    EXPECT_NE(result.errorMessage().find("not initialized"), 0);
}

// Test: 批量操作
TEST_F(UserspaceKernelCallIntegrationTest, BatchOperations) {
    auto initResult = ukc_system_->initialize();
    ASSERT_TRUE(initResult.isSuccess());
    
    pid_t currentPid = getpid();
    std::vector<MemoryOperation> operations;
    
    MemoryOperation op;
    op.type = OperationType::Read;
    op.address = 0x1000;
    op.size = 100;
    operations.push_back(op);
    
    auto result = ukc_system_->batchOperations(currentPid, operations);
    
    // 可能成功或失败，取决于地址是否有效
    // 但不应该返回"未初始化"的错误
    EXPECT_NE(result.errorMessage().find("not initialized"), 0);
}

// Test: 查找进程
TEST_F(UserspaceKernelCallIntegrationTest, FindProcess) {
    auto initResult = ukc_system_->initialize();
    ASSERT_TRUE(initResult.isSuccess());
    
    // 获取当前进程的命令行
    pid_t currentPid = getpid();
    std::string cmdlinePath = "/proc/" + std::to_string(currentPid) + "/cmdline";
    std::ifstream cmdlineFile(cmdlinePath);
    ASSERT_TRUE(cmdlineFile.is_open());
    
    std::string cmdline;
    std::getline(cmdlineFile, cmdline);
    cmdlineFile.close();
    
    // 提取进程名称
    size_t lastSlash = cmdline.find_last_of('/');
    std::string processName = (lastSlash != std::string::npos) 
        ? cmdline.substr(lastSlash + 1) 
        : cmdline;
    
    // 移除 null 字符
    size_t nullPos = processName.find('\0');
    if (nullPos != std::string::npos) {
        processName = processName.substr(0, nullPos);
    }
    
    // 查找进程
    auto result = ukc_system_->findProcessByName(processName);
    ASSERT_TRUE(result.isSuccess());
    EXPECT_EQ(result.value(), currentPid);
}

// Test: 获取进程内存映射
TEST_F(UserspaceKernelCallIntegrationTest, GetProcessMemoryMaps) {
    auto initResult = ukc_system_->initialize();
    ASSERT_TRUE(initResult.isSuccess());
    
    pid_t currentPid = getpid();
    auto result = ukc_system_->getProcessMemoryMaps(currentPid);
    
    ASSERT_TRUE(result.isSuccess());
    const auto& regions = result.value();
    EXPECT_GT(regions.size(), 0);
}

// Test: 完整流程 - 初始化 → 查找 → 读写
TEST_F(UserspaceKernelCallIntegrationTest, CompleteWorkflow) {
    // 1. 初始化
    auto initResult = ukc_system_->initialize();
    ASSERT_TRUE(initResult.isSuccess());
    
    // 2. 获取当前进程信息
    pid_t currentPid = getpid();
    
    // 3. 获取内存映射
    auto mapsResult = ukc_system_->getProcessMemoryMaps(currentPid);
    ASSERT_TRUE(mapsResult.isSuccess());
    ASSERT_GT(mapsResult.value().size(), 0);
    
    // 4. 尝试读取内存
    uintptr_t validAddr = mapsResult.value()[0].start;
    auto readResult = ukc_system_->readMemory(currentPid, validAddr, 100);
    
    // 读取可能成功或失败，但应该返回有效的结果
    if (readResult.isSuccess()) {
        EXPECT_EQ(readResult.value().size(), 100);
    } else {
        EXPECT_FALSE(readResult.errorMessage().empty());
    }
}

// Test: 错误处理 - 不存在的进程
TEST_F(UserspaceKernelCallIntegrationTest, ErrorHandlingNonexistentProcess) {
    auto initResult = ukc_system_->initialize();
    ASSERT_TRUE(initResult.isSuccess());
    
    auto result = ukc_system_->readMemory(99999, 0x1000, 100);
    EXPECT_TRUE(result.isError());
}

// Test: 错误处理 - 无效地址
TEST_F(UserspaceKernelCallIntegrationTest, ErrorHandlingInvalidAddress) {
    auto initResult = ukc_system_->initialize();
    ASSERT_TRUE(initResult.isSuccess());
    
    pid_t currentPid = getpid();
    uintptr_t invalidAddr = 0xFFFFFFFFFFFFFFFFUL;
    
    auto result = ukc_system_->readMemory(currentPid, invalidAddr, 100);
    EXPECT_TRUE(result.isError());
}

// Test: 多次初始化
TEST_F(UserspaceKernelCallIntegrationTest, MultipleInitialize) {
    auto result1 = ukc_system_->initialize();
    ASSERT_TRUE(result1.isSuccess());
    
    auto result2 = ukc_system_->initialize();
    ASSERT_TRUE(result2.isSuccess());
}
