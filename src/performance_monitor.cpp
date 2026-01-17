#include "performance_monitor.h"
#include <algorithm>
#include <numeric>
#include <sstream>
#include <iomanip>

namespace ukc {

std::string PerformanceStats::toString() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "Operation: " << operationName << "\n";
    oss << "  Count: " << operationCount << "\n";
    oss << "  Total Time: " << totalTime.count() << " μs\n";
    oss << "  Min Time: " << minTime.count() << " μs\n";
    oss << "  Max Time: " << maxTime.count() << " μs\n";
    oss << "  Average Time: " << averageTime.count() << " μs\n";
    oss << "  Throughput: " << operationsPerSecond << " ops/sec\n";
    return oss.str();
}

PerformanceMonitor::PerformanceMonitor() = default;

PerformanceMonitor::~PerformanceMonitor() = default;

void PerformanceMonitor::startTimer(const std::string& operationName) {
    TimerEntry& entry = timers_[operationName];
    entry.startTime = std::chrono::steady_clock::now();
}

Result<void> PerformanceMonitor::stopTimer(const std::string& operationName) {
    auto it = timers_.find(operationName);
    if (it == timers_.end()) {
        return Result<void>::error(
            "Timer for operation '" + operationName + "' not started"
        );
    }
    
    auto endTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        endTime - it->second.startTime
    );
    
    it->second.measurements.push_back(duration);
    
    return Result<void>::success();
}

Result<PerformanceStats> PerformanceMonitor::getStats(
    const std::string& operationName
) {
    auto it = timers_.find(operationName);
    if (it == timers_.end() || it->second.measurements.empty()) {
        return Result<PerformanceStats>::error(
            "No measurements for operation '" + operationName + "'"
        );
    }
    
    PerformanceStats stats = calculateStats(
        operationName,
        it->second.measurements
    );
    
    return Result<PerformanceStats>::success(std::move(stats));
}

Result<std::vector<PerformanceStats>> PerformanceMonitor::getAllStats() {
    std::vector<PerformanceStats> allStats;
    
    for (const auto& entry : timers_) {
        if (!entry.second.measurements.empty()) {
            PerformanceStats stats = calculateStats(
                entry.first,
                entry.second.measurements
            );
            allStats.push_back(stats);
        }
    }
    
    return Result<std::vector<PerformanceStats>>::success(std::move(allStats));
}

void PerformanceMonitor::resetStats() {
    timers_.clear();
}

void PerformanceMonitor::resetStats(const std::string& operationName) {
    auto it = timers_.find(operationName);
    if (it != timers_.end()) {
        it->second.measurements.clear();
    }
}

Result<bool> PerformanceMonitor::meetsPerformanceRequirement(
    const std::string& operationName,
    std::chrono::microseconds maxTime
) {
    auto statsResult = getStats(operationName);
    if (statsResult.isError()) {
        return Result<bool>::error(statsResult.errorMessage());
    }
    
    const auto& stats = statsResult.value();
    bool meets = stats.averageTime <= maxTime;
    
    return Result<bool>::success(meets);
}

PerformanceStats PerformanceMonitor::calculateStats(
    const std::string& operationName,
    const std::vector<std::chrono::microseconds>& measurements
) {
    PerformanceStats stats;
    stats.operationName = operationName;
    stats.operationCount = measurements.size();
    
    if (measurements.empty()) {
        return stats;
    }
    
    // 计算总时间
    stats.totalTime = std::accumulate(
        measurements.begin(),
        measurements.end(),
        std::chrono::microseconds{0}
    );
    
    // 计算最小和最大时间
    auto minMaxIt = std::minmax_element(measurements.begin(), measurements.end());
    stats.minTime = *minMaxIt.first;
    stats.maxTime = *minMaxIt.second;
    
    // 计算平均时间
    stats.averageTime = std::chrono::microseconds(
        stats.totalTime.count() / measurements.size()
    );
    
    // 计算吞吐量（操作/秒）
    if (stats.totalTime.count() > 0) {
        stats.operationsPerSecond = 
            (measurements.size() * 1000000.0) / stats.totalTime.count();
    }
    
    return stats;
}

} // namespace ukc
