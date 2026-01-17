#include <gtest/gtest.h>
#include "performance_monitor.h"
#include <thread>
#include <chrono>

using namespace ukc;

class PerformanceMonitorTest : public ::testing::Test {
protected:
    PerformanceMonitor monitor;
};

// Test: 基本计时
TEST_F(PerformanceMonitorTest, BasicTiming) {
    monitor.startTimer("test_operation");
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    auto result = monitor.stopTimer("test_operation");
    
    ASSERT_TRUE(result.isSuccess());
    
    auto statsResult = monitor.getStats("test_operation");
    ASSERT_TRUE(statsResult.isSuccess());
    
    const auto& stats = statsResult.value();
    EXPECT_EQ(stats.operationCount, 1);
    EXPECT_GE(stats.totalTime.count(), 10000);  // 至少 10ms
}

// Test: 多次计时
TEST_F(PerformanceMonitorTest, MultipleMeasurements) {
    for (int i = 0; i < 5; ++i) {
        monitor.startTimer("test_operation");
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        auto result = monitor.stopTimer("test_operation");
        ASSERT_TRUE(result.isSuccess());
    }
    
    auto statsResult = monitor.getStats("test_operation");
    ASSERT_TRUE(statsResult.isSuccess());
    
    const auto& stats = statsResult.value();
    EXPECT_EQ(stats.operationCount, 5);
    EXPECT_GT(stats.averageTime.count(), 0);
    EXPECT_LE(stats.minTime, stats.maxTime);
}

// Test: 停止未启动的计时器
TEST_F(PerformanceMonitorTest, StopUnstartedTimer) {
    auto result = monitor.stopTimer("nonexistent_operation");
    ASSERT_TRUE(result.isError());
}

// Test: 获取不存在的统计信息
TEST_F(PerformanceMonitorTest, GetNonexistentStats) {
    auto result = monitor.getStats("nonexistent_operation");
    ASSERT_TRUE(result.isError());
}

// Test: 获取所有统计信息
TEST_F(PerformanceMonitorTest, GetAllStats) {
    // 添加多个操作
    for (int i = 0; i < 3; ++i) {
        std::string opName = "operation_" + std::to_string(i);
        monitor.startTimer(opName);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        monitor.stopTimer(opName);
    }
    
    auto result = monitor.getAllStats();
    ASSERT_TRUE(result.isSuccess());
    
    const auto& allStats = result.value();
    EXPECT_EQ(allStats.size(), 3);
}

// Test: 重置统计信息
TEST_F(PerformanceMonitorTest, ResetStats) {
    monitor.startTimer("test_operation");
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    monitor.stopTimer("test_operation");
    
    auto beforeReset = monitor.getStats("test_operation");
    ASSERT_TRUE(beforeReset.isSuccess());
    EXPECT_EQ(beforeReset.value().operationCount, 1);
    
    monitor.resetStats("test_operation");
    
    auto afterReset = monitor.getStats("test_operation");
    ASSERT_TRUE(afterReset.isError());
}

// Test: 重置所有统计信息
TEST_F(PerformanceMonitorTest, ResetAllStats) {
    for (int i = 0; i < 3; ++i) {
        std::string opName = "operation_" + std::to_string(i);
        monitor.startTimer(opName);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        monitor.stopTimer(opName);
    }
    
    auto beforeReset = monitor.getAllStats();
    ASSERT_TRUE(beforeReset.isSuccess());
    EXPECT_EQ(beforeReset.value().size(), 3);
    
    monitor.resetStats();
    
    auto afterReset = monitor.getAllStats();
    ASSERT_TRUE(afterReset.isSuccess());
    EXPECT_EQ(afterReset.value().size(), 0);
}

// Test: 性能要求检查 - 满足
TEST_F(PerformanceMonitorTest, MeetsPerformanceRequirement) {
    monitor.startTimer("fast_operation");
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    monitor.stopTimer("fast_operation");
    
    auto result = monitor.meetsPerformanceRequirement(
        "fast_operation",
        std::chrono::milliseconds(100)
    );
    
    ASSERT_TRUE(result.isSuccess());
    EXPECT_TRUE(result.value());
}

// Test: 性能要求检查 - 不满足
TEST_F(PerformanceMonitorTest, DoesNotMeetPerformanceRequirement) {
    monitor.startTimer("slow_operation");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    monitor.stopTimer("slow_operation");
    
    auto result = monitor.meetsPerformanceRequirement(
        "slow_operation",
        std::chrono::milliseconds(10)
    );
    
    ASSERT_TRUE(result.isSuccess());
    EXPECT_FALSE(result.value());
}

// Test: 统计信息的字符串表示
TEST_F(PerformanceMonitorTest, StatsToString) {
    monitor.startTimer("test_operation");
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    monitor.stopTimer("test_operation");
    
    auto statsResult = monitor.getStats("test_operation");
    ASSERT_TRUE(statsResult.isSuccess());
    
    const auto& stats = statsResult.value();
    std::string str = stats.toString();
    
    EXPECT_FALSE(str.empty());
    EXPECT_NE(str.find("test_operation"), std::string::npos);
    EXPECT_NE(str.find("Count:"), std::string::npos);
}

// Test: 最小和最大时间
TEST_F(PerformanceMonitorTest, MinMaxTime) {
    // 添加不同长度的操作
    std::vector<int> sleepTimes = {1, 5, 3, 10, 2};
    
    for (int sleepMs : sleepTimes) {
        monitor.startTimer("variable_operation");
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepMs));
        monitor.stopTimer("variable_operation");
    }
    
    auto statsResult = monitor.getStats("variable_operation");
    ASSERT_TRUE(statsResult.isSuccess());
    
    const auto& stats = statsResult.value();
    EXPECT_LE(stats.minTime, stats.averageTime);
    EXPECT_LE(stats.averageTime, stats.maxTime);
}

// Test: 吞吐量计算
TEST_F(PerformanceMonitorTest, ThroughputCalculation) {
    for (int i = 0; i < 10; ++i) {
        monitor.startTimer("throughput_test");
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        monitor.stopTimer("throughput_test");
    }
    
    auto statsResult = monitor.getStats("throughput_test");
    ASSERT_TRUE(statsResult.isSuccess());
    
    const auto& stats = statsResult.value();
    EXPECT_GT(stats.operationsPerSecond, 0);
}
