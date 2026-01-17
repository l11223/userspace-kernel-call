#include <gtest/gtest.h>
#include "process_manager.h"
#include <unistd.h>
#include <sys/types.h>

using namespace ukc;

class ProcessManagerTest : public ::testing::Test {
protected:
    ProcessManager pm;
};

// Test: 查找当前进程
TEST_F(ProcessManagerTest, FindCurrentProcess) {
    pid_t currentPid = getpid();
    
    // 获取当前进程的命令行
    std::string cmdlinePath = "/proc/" + std::to_string(currentPid) + "/cmdline";
    std::ifstream cmdlineFile(cmdlinePath);
    ASSERT_TRUE(cmdlineFile.is_open());
    
    std::string cmdline;
    std::getline(cmdlineFile, cmdline);
    cmdlineFile.close();
    
    // 提取进程名称（第一个参数）
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
    auto result = pm.findProcessByName(processName);
    ASSERT_TRUE(result.isSuccess());
    EXPECT_EQ(result.value(), currentPid);
}

// Test: 查找不存在的进程
TEST_F(ProcessManagerTest, FindNonexistentProcess) {
    auto result = pm.findProcessByName("nonexistent_process_xyz_12345");
    ASSERT_TRUE(result.isError());
    EXPECT_FALSE(result.errorMessage().empty());
}

// Test: 验证当前进程存活
TEST_F(ProcessManagerTest, IsProcessAlive) {
    pid_t currentPid = getpid();
    EXPECT_TRUE(pm.isProcessAlive(currentPid));
}

// Test: 验证不存在的进程不存活
TEST_F(ProcessManagerTest, IsProcessNotAlive) {
    // 使用一个不太可能存在的 PID
    EXPECT_FALSE(pm.isProcessAlive(99999));
}

// Test: 获取当前进程的内存映射
TEST_F(ProcessManagerTest, GetMemoryMaps) {
    pid_t currentPid = getpid();
    auto result = pm.getMemoryMaps(currentPid);
    
    ASSERT_TRUE(result.isSuccess());
    const auto& regions = result.value();
    EXPECT_GT(regions.size(), 0);
    
    // 验证内存区域的基本属性
    for (const auto& region : regions) {
        EXPECT_LT(region.start, region.end);
        EXPECT_FALSE(region.permissions.empty());
    }
}

// Test: 获取不存在进程的内存映射
TEST_F(ProcessManagerTest, GetMemoryMapsNonexistent) {
    auto result = pm.getMemoryMaps(99999);
    ASSERT_TRUE(result.isError());
}

// Test: 验证有效地址
TEST_F(ProcessManagerTest, IsValidAddress) {
    pid_t currentPid = getpid();
    
    // 获取内存映射
    auto mapsResult = pm.getMemoryMaps(currentPid);
    ASSERT_TRUE(mapsResult.isSuccess());
    
    const auto& regions = mapsResult.value();
    ASSERT_GT(regions.size(), 0);
    
    // 第一个区域的起始地址应该是有效的
    uintptr_t validAddr = regions[0].start;
    EXPECT_TRUE(pm.isValidAddress(currentPid, validAddr));
}

// Test: 验证无效地址
TEST_F(ProcessManagerTest, IsInvalidAddress) {
    pid_t currentPid = getpid();
    
    // 使用一个极端的地址
    uintptr_t invalidAddr = 0xFFFFFFFFFFFFFFFFUL;
    EXPECT_FALSE(pm.isValidAddress(currentPid, invalidAddr));
}

// Test: 验证不存在进程的地址
TEST_F(ProcessManagerTest, IsValidAddressNonexistentProcess) {
    EXPECT_FALSE(pm.isValidAddress(99999, 0x1000));
}

// Test: 内存映射解析 - 边界情况
TEST_F(ProcessManagerTest, ParseMemoryMapsEdgeCases) {
    pid_t currentPid = getpid();
    auto result = pm.getMemoryMaps(currentPid);
    
    ASSERT_TRUE(result.isSuccess());
    const auto& regions = result.value();
    
    // 验证所有区域都有合理的地址范围
    for (const auto& region : regions) {
        EXPECT_GT(region.end, region.start);
        EXPECT_GT(region.end - region.start, 0);
    }
}

// Test: 内存映射排序
TEST_F(ProcessManagerTest, MemoryMapsOrdering) {
    pid_t currentPid = getpid();
    auto result = pm.getMemoryMaps(currentPid);
    
    ASSERT_TRUE(result.isSuccess());
    const auto& regions = result.value();
    
    // 验证内存区域按地址排序
    for (size_t i = 1; i < regions.size(); ++i) {
        EXPECT_LE(regions[i-1].start, regions[i].start);
    }
}
