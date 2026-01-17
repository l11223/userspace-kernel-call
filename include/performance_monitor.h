#ifndef USERSPACE_KERNEL_CALL_PERFORMANCE_MONITOR_H
#define USERSPACE_KERNEL_CALL_PERFORMANCE_MONITOR_H

#include "result.h"
#include <chrono>
#include <string>
#include <vector>
#include <map>

namespace ukc {

/**
 * 性能统计信息
 */
struct PerformanceStats {
    std::string operationName;
    size_t operationCount = 0;
    
    // 时间统计（微秒）
    std::chrono::microseconds totalTime{0};
    std::chrono::microseconds minTime{0};
    std::chrono::microseconds maxTime{0};
    std::chrono::microseconds averageTime{0};
    
    // 吞吐量统计
    double operationsPerSecond = 0.0;
    
    /**
     * 获取统计信息的字符串表示
     */
    std::string toString() const;
};

/**
 * 性能监控器
 * 用于监控和统计系统性能
 */
class PerformanceMonitor {
public:
    PerformanceMonitor();
    ~PerformanceMonitor();
    
    /**
     * 开始计时
     */
    void startTimer(const std::string& operationName);
    
    /**
     * 停止计时并记录
     */
    Result<void> stopTimer(const std::string& operationName);
    
    /**
     * 获取操作的性能统计
     */
    Result<PerformanceStats> getStats(const std::string& operationName);
    
    /**
     * 获取所有操作的性能统计
     */
    Result<std::vector<PerformanceStats>> getAllStats();
    
    /**
     * 重置所有统计信息
     */
    void resetStats();
    
    /**
     * 重置特定操作的统计信息
     */
    void resetStats(const std::string& operationName);
    
    /**
     * 检查操作是否满足性能要求
     */
    Result<bool> meetsPerformanceRequirement(
        const std::string& operationName,
        std::chrono::microseconds maxTime
    );

private:
    struct TimerEntry {
        std::chrono::steady_clock::time_point startTime;
        std::vector<std::chrono::microseconds> measurements;
    };
    
    std::map<std::string, TimerEntry> timers_;
    
    /**
     * 计算统计信息
     */
    PerformanceStats calculateStats(
        const std::string& operationName,
        const std::vector<std::chrono::microseconds>& measurements
    );
};

} // namespace ukc

#endif // USERSPACE_KERNEL_CALL_PERFORMANCE_MONITOR_H
