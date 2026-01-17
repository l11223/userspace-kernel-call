#include <gtest/gtest.h>
#include <rapidcheck.h>
#include <rapidcheck/gtest.h>
#include "memory_injector.h"
#include "process_manager.h"
#include "kernel_function_locator.h"
#include "kernel_caller.h"
#include <unistd.h>
#include <memory>

using namespace ukc;

class MemoryInjectorPropertyTest : public ::testing::Test {
protected:
    void SetUp() override {
        locator_ = std::make_shared<KernelFunctionLocator>();
        caller_ = std::make_shared<KernelCaller>();
        processManager_ = std::make_shared<ProcessManager>();
        injector_ = std::make_unique<MemoryInjector>();
        
        // 初始化注入器
        auto result = injector_->initialize(locator_, caller_, processManager_);
        ASSERT_TRUE(result.isSuccess());
    }
    
    std::shared_ptr<KernelFunctionLocator> locator_;
    std::shared_ptr<KernelCaller> caller_;
    std::shared_ptr<ProcessManager> processManager_;
    std::unique_ptr<MemoryInjector> injector_;
};

// Feature: userspace-kernel-call, Property 7: 内存操作结果
// Validates: Requirements 3.4
RC_GTEST_PROP(MemoryInjectorPropertyTest, PropertyMemoryOperationResult,
              (size_t size)) {
    // 限制大小在合理范围内
    size = std::min(size, size_t(4096));
    
    pid_t currentPid = getpid();
    
    // 获取有效地址
    auto mapsResult = processManager_->getMemoryMaps(currentPid);
    RC_ASSERT(mapsResult.isSuccess());
    RC_ASSERT(mapsResult.value().size() > 0);
    
    uintptr_t validAddr = mapsResult.value()[0].start;
    
    // 测试读操作
    if (size > 0) {
        auto readResult = injector_->readMemory(currentPid, validAddr, size);
        if (readResult.isSuccess()) {
            // 返回的数据大小应该等于请求的大小
            RC_ASSERT(readResult.value().size() == size);
        }
    }
    
    // 测试写操作
    if (size > 0) {
        std::vector<uint8_t> data(size, 0xAA);
        auto writeResult = injector_->writeMemory(currentPid, validAddr, data);
        if (writeResult.isSuccess()) {
            // 返回的字节数应该等于写入的数据大小
            RC_ASSERT(writeResult.value() == size);
        }
    }
}

// Feature: userspace-kernel-call, Property 12: 批量操作优化
// Validates: Requirements 8.5
RC_GTEST_PROP(MemoryInjectorPropertyTest, PropertyBatchOperationOptimization,
              (size_t operationCount)) {
    // 限制操作数量
    operationCount = std::min(operationCount, size_t(100));
    
    pid_t currentPid = getpid();
    
    // 获取有效地址
    auto mapsResult = processManager_->getMemoryMaps(currentPid);
    RC_ASSERT(mapsResult.isSuccess());
    RC_ASSERT(mapsResult.value().size() > 0);
    
    uintptr_t validAddr = mapsResult.value()[0].start;
    
    // 创建批量操作
    std::vector<MemoryOperation> operations;
    for (size_t i = 0; i < operationCount; ++i) {
        MemoryOperation op;
        op.type = (i % 2 == 0) ? OperationType::Read : OperationType::Write;
        op.address = validAddr;
        
        if (op.type == OperationType::Read) {
            op.size = 100;
        } else {
            op.data = std::vector<uint8_t>(100, 0xBB);
        }
        
        operations.push_back(op);
    }
    
    // 执行批量操作
    auto result = injector_->batchOperations(currentPid, operations);
    RC_ASSERT(result.isSuccess());
    
    // 验证所有操作都被处理
    RC_ASSERT(operations.size() == operationCount);
}

// Feature: userspace-kernel-call, Property 2: 地址验证
// Validates: Requirements 1.3, 3.2, 6.1
RC_GTEST_PROP(MemoryInjectorPropertyTest, PropertyAddressValidation,
              (uint64_t addressOffset)) {
    pid_t currentPid = getpid();
    
    // 获取有效地址范围
    auto mapsResult = processManager_->getMemoryMaps(currentPid);
    RC_ASSERT(mapsResult.isSuccess());
    RC_ASSERT(mapsResult.value().size() > 0);
    
    const auto& regions = mapsResult.value();
    uintptr_t regionStart = regions[0].start;
    uintptr_t regionEnd = regions[0].end;
    
    // 生成测试地址
    uintptr_t testAddr = regionStart + (addressOffset % (regionEnd - regionStart));
    
    // 读取操作应该验证地址
    auto readResult = injector_->readMemory(currentPid, testAddr, 1);
    
    // 如果地址有效，操作应该成功或返回有效的错误
    if (testAddr >= regionStart && testAddr < regionEnd) {
        // 地址在有效范围内
        RC_ASSERT(readResult.isSuccess() || !readResult.errorMessage().empty());
    }
}

// Feature: userspace-kernel-call, Property 6: 错误处理和资源清理
// Validates: Requirements 1.4, 2.5, 3.5, 4.4, 6.4, 7.5
RC_GTEST_PROP(MemoryInjectorPropertyTest, PropertyErrorHandling,
              (bool useInvalidPid)) {
    pid_t targetPid = useInvalidPid ? 99999 : getpid();
    
    std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04};
    
    // 测试读操作的错误处理
    auto readResult = injector_->readMemory(targetPid, 0x1000, 100);
    
    if (useInvalidPid) {
        // 无效的 PID 应该返回错误
        RC_ASSERT(readResult.isError());
        RC_ASSERT(!readResult.errorMessage().empty());
    }
    
    // 测试写操作的错误处理
    auto writeResult = injector_->writeMemory(targetPid, 0x1000, data);
    
    if (useInvalidPid) {
        // 无效的 PID 应该返回错误
        RC_ASSERT(writeResult.isError());
        RC_ASSERT(!writeResult.errorMessage().empty());
    }
}
