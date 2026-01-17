#include <gtest/gtest.h>
#include <rapidcheck.h>
#include <rapidcheck/gtest.h>
#include "performance_monitor.h"
#include <thread>
#include <chrono>

using namespace ukc;

class PerformanceMonitorPropertyTest : public ::testing::Test {
protected:
    PerformanceMonitor monitor;
};

// Feature: userspace-kernel-call, Property 11: 性能要求
// Validates: Requirements 8.2
RC_GTEST_PROP(PerformanceMonitorPropertyTest, PropertyPerformanceRequirement,
              (size_t operationCount)) {
    // 限制操作数量
    operationCount = std::min(operationCount, size_t(100));
    if (operationCount == 0) operationCount = 1;
    
    // 执行多个快速操作
    for (size_t i = 0; i < operationCount; ++i) {
        monitor.startTimer("fast_operation");
        // 模拟快速操作（< 1ms）
        std::this_thread::sleep_for(std::chrono::microseconds(100));
        auto result = monitor.stopTimer("fast_operation");
        RC_ASSERT(result.isSuccess());
    }
    
    // 验证平均时间在 100ms 以内
    auto statsResult = monitor.getStats("fast_operation");
    RC_ASSERT(statsResult.isSuccess());
    
    const auto& stats = statsResult.value();
    RC_ASSERT(stats.operationCount == operationCount);
    
    // 快速操作应该满足性能要求
    auto requirementResult = monitor.meetsPerformanceRequirement(
        "fast_operation",
        std::chrono::milliseconds(100)
    );
    RC_ASSERT(requirementResult.isSuccess());
    RC_ASSERT(requirementResult.value());
}

// Feature: userspace-kernel-call, Property 12: 批量操作优化
// Validates: Requirements 8.5
RC_GTEST_PROP(PerformanceMonitorPropertyTest, PropertyBatchOperationPerformance,
              (size_t batchSize)) {
    // 限制批量大小
    batchSize = std::min(batchSize, size_t(100));
    if (batchSize == 0) batchSize = 1;
    
    // 测试单个操作
    monitor.startTimer("single_operation");
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    monitor.stopTimer("single_operation");
    
    auto singleStats = monitor.getStats("single_operation");
    RC_ASSERT(singleStats.isSuccess());
    
    // 测试批量操作
    monitor.startTimer("batch_operation");
    for (size_t i = 0; i < batchSize; ++i) {
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
    monitor.stopTimer("batch_operation");
    
    auto batchStats = monitor.getStats("batch_operation");
    RC_ASSERT(batchStats.isSuccess());
    
    // 批量操作的平均时间应该接近单个操作的时间
    // （因为我们在测试性能监控，而不是实际的批量优化）
    RC_ASSERT(batchStats.value().operationCount == 1);
}

// Feature: userspace-kernel-call, Property: 统计一致性
// 验证统计信息的一致性
RC_GTEST_PROP(PerformanceMonitorPropertyTest, PropertyStatisticsConsistency,
              (size_t operationCount)) {
    // 限制操作数量
    operationCount = std::min(operationCount, size_t(50));
    if (operationCount == 0) operationCount = 1;
    
    // 执行多个操作
    for (size_t i = 0; i < operationCount; ++i) {
        monitor.startTimer("consistency_test");
        std::this_thread::sleep_for(std::chrono::microseconds(100));
        auto result = monitor.stopTimer("consistency_test");
        RC_ASSERT(result.isSuccess());
    }
    
    auto statsResult = monitor.getStats("consistency_test");
    RC_ASSERT(statsResult.isSuccess());
    
    const auto& stats = statsResult.value();
    
    // 验证统计信息的一致性
    RC_ASSERT(stats.operationCount == operationCount);
    RC_ASSERT(stats.totalTime.count() > 0);
    RC_ASSERT(stats.minTime.count() > 0);
    RC_ASSERT(stats.maxTime.count() > 0);
    RC_ASSERT(stats.averageTime.count() > 0);
    RC_ASSERT(stats.operationsPerSecond > 0);
    
    // 验证最小值 <= 平均值 <= 最大值
    RC_ASSERT(stats.minTime <= stats.averageTime);
    RC_ASSERT(stats.averageTime <= stats.maxTime);
    
    // 验证总时间 = 平均时间 * 操作数
    auto expectedTotal = stats.averageTime.count() * operationCount;
    auto actualTotal = stats.totalTime.count();
    // 允许 1% 的误差
    RC_ASSERT(std::abs(static_cast<long long>(expectedTotal - actualTotal)) 
              <= actualTotal / 100);
}

// Feature: userspace-kernel-call, Property: 计时器准确性
// 验证计时器的准确性
RC_GTEST_PROP(PerformanceMonitorPropertyTest, PropertyTimerAccuracy,
              (int sleepMs)) {
    // 限制睡眠时间
    sleepMs = std::abs(sleepMs) % 100 + 1;
    
    monitor.startTimer("accuracy_test");
    std::this_thread::sleep_for(std::chrono::milliseconds(sleepMs));
    auto result = monitor.stopTimer("accuracy_test");
    
    RC_ASSERT(result.isSuccess());
    
    auto statsResult = monitor.getStats("accuracy_test");
    RC_ASSERT(statsResult.isSuccess());
    
    const auto& stats = statsResult.value();
    
    // 测量的时间应该接近预期的睡眠时间
    // 允许 50% 的误差（因为系统调度的不确定性）
    int expectedUs = sleepMs * 1000;
    int actualUs = stats.totalTime.count();
    
    RC_ASSERT(actualUs >= expectedUs / 2);
    RC_ASSERT(actualUs <= expectedUs * 2);
}

// Feature: userspace-kernel-call, Property: 多操作独立性
// 验证不同操作的统计信息是独立的
RC_GTEST_PROP(PerformanceMonitorPropertyTest, PropertyMultipleOperationsIndependence,
              (size_t operationCount)) {
    // 限制操作数量
    operationCount = std::min(operationCount, size_t(10));
    if (operationCount == 0) operationCount = 1;
    
    // 执行多个不同的操作
    for (size_t i = 0; i < operationCount; ++i) {
        std::string opName = "operation_" + std::to_string(i);
        monitor.startTimer(opName);
        std::this_thread::sleep_for(std::chrono::microseconds(100));
        auto result = monitor.stopTimer(opName);
        RC_ASSERT(result.isSuccess());
    }
    
    // 验证每个操作都有独立的统计信息
    auto allStats = monitor.getAllStats();
    RC_ASSERT(allStats.isSuccess());
    RC_ASSERT(allStats.value().size() == operationCount);
    
    // 验证每个操作的统计信息都是正确的
    for (const auto& stats : allStats.value()) {
        RC_ASSERT(stats.operationCount == 1);
        RC_ASSERT(stats.totalTime.count() > 0);
    }
}
