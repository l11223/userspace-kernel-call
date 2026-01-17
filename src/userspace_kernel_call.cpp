#include "userspace_kernel_call.h"
#include "kernel_function_locator.h"
#include "kernel_caller.h"
#include "process_manager.h"
#include "memory_injector.h"

namespace ukc {

UserspaceKernelCall::UserspaceKernelCall() = default;

UserspaceKernelCall::~UserspaceKernelCall() = default;

Result<void> UserspaceKernelCall::initialize() {
    if (initialized_) {
        return Result<void>::success();
    }
    
    // 创建组件
    locator_ = std::make_shared<KernelFunctionLocator>();
    caller_ = std::make_shared<KernelCaller>();
    processManager_ = std::make_shared<ProcessManager>();
    injector_ = std::make_shared<MemoryInjector>();
    
    // 初始化定位器
    auto locatorResult = locator_->initialize();
    if (locatorResult.isError()) {
        return locatorResult;
    }
    
    // 初始化调用器
    auto callerResult = caller_->initialize();
    if (callerResult.isError()) {
        return callerResult;
    }
    
    // 初始化注入器
    auto injectorResult = injector_->initialize(locator_, caller_, processManager_);
    if (injectorResult.isError()) {
        return injectorResult;
    }
    
    initialized_ = true;
    return Result<void>::success();
}

Result<std::vector<uint8_t>> UserspaceKernelCall::readMemory(
    pid_t targetPid,
    uintptr_t address,
    size_t size
) {
    if (!initialized_) {
        return Result<std::vector<uint8_t>>::error("System not initialized");
    }
    
    return injector_->readMemory(targetPid, address, size);
}

Result<size_t> UserspaceKernelCall::writeMemory(
    pid_t targetPid,
    uintptr_t address,
    const std::vector<uint8_t>& data
) {
    if (!initialized_) {
        return Result<size_t>::error("System not initialized");
    }
    
    return injector_->writeMemory(targetPid, address, data);
}

Result<void> UserspaceKernelCall::batchOperations(
    pid_t targetPid,
    std::vector<MemoryOperation>& operations
) {
    if (!initialized_) {
        return Result<void>::error("System not initialized");
    }
    
    return injector_->batchOperations(targetPid, operations);
}

Result<pid_t> UserspaceKernelCall::findProcessByName(const std::string& processName) {
    if (!initialized_) {
        return Result<pid_t>::error("System not initialized");
    }
    
    return processManager_->findProcessByName(processName);
}

Result<std::vector<MemoryRegion>> UserspaceKernelCall::getProcessMemoryMaps(pid_t pid) {
    if (!initialized_) {
        return Result<std::vector<MemoryRegion>>::error("System not initialized");
    }
    
    return processManager_->getMemoryMaps(pid);
}

} // namespace ukc
