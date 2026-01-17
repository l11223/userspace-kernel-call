#include <gtest/gtest.h>
#include "memory_injector.h"
#include "process_manager.h"
#include "kernel_function_locator.h"
#include "kernel_caller.h"
#include <unistd.h>
#include <memory>

using namespace ukc;

class MemoryInjectorTest : public ::testing::Test {
protected:
    void SetUp() override {
        locator_ = std::make_shared<KernelFunctionLocator>();
        caller_ = std::make_shared<KernelCaller>();
        processManager_ = std::make_shared<ProcessManager>();
        injector_ = std::make_unique<MemoryInjector>();
    }
    
    std::shared_ptr<KernelFunctionLocator> locator_;
    std::shared_ptr<KernelCaller> caller_;
    std::shared_ptr<ProcessManager> processManager_;
    std::unique_ptr<MemoryInjector> injector_;
};

// Test: 初始化注入器
TEST_F(MemoryInjectorTest, Initialize) {
    auto result = injector_->initialize(locator_, caller_, processManager_);
    EXPECT_TRUE(result.isSuccess());
}

// Test: 初始化失败 - 无效的依赖
TEST_F(MemoryInjectorTest, InitializeWithInvalidDependencies) {
    auto result = injector_->initialize(nullptr, caller_, processManager_);
    EXPECT_TRUE(result.isError());
}

// Test: 未初始化时读取内存
TEST_F(MemoryInjectorTest, ReadMemoryWithoutInitialize) {
    pid_t currentPid = getpid();
    auto result = injector_->readMemory(currentPid, 0x1000, 100);
    EXPECT_TRUE(result.isError());
}

// Test: 读取不存在进程的内存
TEST_F(MemoryInjectorTest, ReadMemoryNonexistentProcess) {
    auto initResult = injector_->initialize(locator_, caller_, processManager_);
    ASSERT_TRUE(initResult.isSuccess());
    
    auto result = injector_->readMemory(99999, 0x1000, 100);
    EXPECT_TRUE(result.isError());
}

// Test: 读取无效地址
TEST_F(MemoryInjectorTest, ReadMemoryInvalidAddress) {
    auto initResult = injector_->initialize(locator_, caller_, processManager_);
    ASSERT_TRUE(initResult.isSuccess());
    
    pid_t currentPid = getpid();
    uintptr_t invalidAddr = 0xFFFFFFFFFFFFFFFFUL;
    
    auto result = injector_->readMemory(currentPid, invalidAddr, 100);
    EXPECT_TRUE(result.isError());
}

// Test: 读取零字节
TEST_F(MemoryInjectorTest, ReadMemoryZeroBytes) {
    auto initResult = injector_->initialize(locator_, caller_, processManager_);
    ASSERT_TRUE(initResult.isSuccess());
    
    pid_t currentPid = getpid();
    auto result = injector_->readMemory(currentPid, 0x1000, 0);
    
    EXPECT_TRUE(result.isSuccess());
    EXPECT_EQ(result.value().size(), 0);
}

// Test: 写入内存
TEST_F(MemoryInjectorTest, WriteMemory) {
    auto initResult = injector_->initialize(locator_, caller_, processManager_);
    ASSERT_TRUE(initResult.isSuccess());
    
    pid_t currentPid = getpid();
    std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04};
    
    // 获取有效地址
    auto mapsResult = processManager_->getMemoryMaps(currentPid);
    ASSERT_TRUE(mapsResult.isSuccess());
    ASSERT_GT(mapsResult.value().size(), 0);
    
    uintptr_t validAddr = mapsResult.value()[0].start;
    auto result = injector_->writeMemory(currentPid, validAddr, data);
    
    EXPECT_TRUE(result.isSuccess());
    EXPECT_EQ(result.value(), data.size());
}

// Test: 写入不存在进程的内存
TEST_F(MemoryInjectorTest, WriteMemoryNonexistentProcess) {
    auto initResult = injector_->initialize(locator_, caller_, processManager_);
    ASSERT_TRUE(initResult.isSuccess());
    
    std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04};
    auto result = injector_->writeMemory(99999, 0x1000, data);
    EXPECT_TRUE(result.isError());
}

// Test: 写入无效地址
TEST_F(MemoryInjectorTest, WriteMemoryInvalidAddress) {
    auto initResult = injector_->initialize(locator_, caller_, processManager_);
    ASSERT_TRUE(initResult.isSuccess());
    
    pid_t currentPid = getpid();
    uintptr_t invalidAddr = 0xFFFFFFFFFFFFFFFFUL;
    std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04};
    
    auto result = injector_->writeMemory(currentPid, invalidAddr, data);
    EXPECT_TRUE(result.isError());
}

// Test: 写入零字节
TEST_F(MemoryInjectorTest, WriteMemoryZeroBytes) {
    auto initResult = injector_->initialize(locator_, caller_, processManager_);
    ASSERT_TRUE(initResult.isSuccess());
    
    pid_t currentPid = getpid();
    std::vector<uint8_t> data;
    
    auto result = injector_->writeMemory(currentPid, 0x1000, data);
    EXPECT_TRUE(result.isSuccess());
    EXPECT_EQ(result.value(), 0);
}

// Test: 批量操作
TEST_F(MemoryInjectorTest, BatchOperations) {
    auto initResult = injector_->initialize(locator_, caller_, processManager_);
    ASSERT_TRUE(initResult.isSuccess());
    
    pid_t currentPid = getpid();
    
    // 获取有效地址
    auto mapsResult = processManager_->getMemoryMaps(currentPid);
    ASSERT_TRUE(mapsResult.isSuccess());
    ASSERT_GT(mapsResult.value().size(), 0);
    
    uintptr_t validAddr = mapsResult.value()[0].start;
    
    std::vector<MemoryOperation> operations;
    
    // 添加读操作
    MemoryOperation readOp;
    readOp.type = OperationType::Read;
    readOp.address = validAddr;
    readOp.size = 100;
    operations.push_back(readOp);
    
    // 添加写操作
    MemoryOperation writeOp;
    writeOp.type = OperationType::Write;
    writeOp.address = validAddr;
    writeOp.data = {0x01, 0x02, 0x03, 0x04};
    operations.push_back(writeOp);
    
    auto result = injector_->batchOperations(currentPid, operations);
    EXPECT_TRUE(result.isSuccess());
    
    // 验证操作结果
    EXPECT_TRUE(operations[0].success);
    EXPECT_TRUE(operations[1].success);
}

// Test: 批量操作 - 不存在的进程
TEST_F(MemoryInjectorTest, BatchOperationsNonexistentProcess) {
    auto initResult = injector_->initialize(locator_, caller_, processManager_);
    ASSERT_TRUE(initResult.isSuccess());
    
    std::vector<MemoryOperation> operations;
    MemoryOperation op;
    op.type = OperationType::Read;
    op.address = 0x1000;
    op.size = 100;
    operations.push_back(op);
    
    auto result = injector_->batchOperations(99999, operations);
    EXPECT_TRUE(result.isError());
}

// Test: 批量操作 - 空操作列表
TEST_F(MemoryInjectorTest, BatchOperationsEmpty) {
    auto initResult = injector_->initialize(locator_, caller_, processManager_);
    ASSERT_TRUE(initResult.isSuccess());
    
    pid_t currentPid = getpid();
    std::vector<MemoryOperation> operations;
    
    auto result = injector_->batchOperations(currentPid, operations);
    EXPECT_TRUE(result.isSuccess());
}

// Test: 批量操作 - 无效地址
TEST_F(MemoryInjectorTest, BatchOperationsInvalidAddress) {
    auto initResult = injector_->initialize(locator_, caller_, processManager_);
    ASSERT_TRUE(initResult.isSuccess());
    
    pid_t currentPid = getpid();
    
    std::vector<MemoryOperation> operations;
    MemoryOperation op;
    op.type = OperationType::Read;
    op.address = 0xFFFFFFFFFFFFFFFFUL;
    op.size = 100;
    operations.push_back(op);
    
    auto result = injector_->batchOperations(currentPid, operations);
    EXPECT_TRUE(result.isSuccess());
    
    // 操作应该标记为失败
    EXPECT_FALSE(operations[0].success);
    EXPECT_FALSE(operations[0].errorMessage.empty());
}
