#include <gtest/gtest.h>
#include "stealth_verifier.h"
#include <unistd.h>

using namespace ukc;

class StealthVerifierTest : public ::testing::Test {
protected:
    StealthVerifier verifier;
};

// Test: 捕获系统状态
TEST_F(StealthVerifierTest, CaptureSystemState) {
    auto result = verifier.captureSystemState();
    ASSERT_TRUE(result.isSuccess());
    
    const auto& snapshot = result.value();
    EXPECT_GT(snapshot.loadedModules.size(), 0);
    EXPECT_GT(snapshot.devFiles.size(), 0);
    EXPECT_GT(snapshot.openFileDescriptors, 0);
    EXPECT_GT(snapshot.allocatedMemory, 0);
}

// Test: 检查新模块加载
TEST_F(StealthVerifierTest, HasNewModulesLoaded) {
    auto beforeResult = verifier.captureSystemState();
    ASSERT_TRUE(beforeResult.isSuccess());
    
    auto afterResult = verifier.captureSystemState();
    ASSERT_TRUE(afterResult.isSuccess());
    
    // 在没有加载新模块的情况下，应该返回 false
    auto result = verifier.hasNewModulesLoaded(
        beforeResult.value(),
        afterResult.value()
    );
    
    ASSERT_TRUE(result.isSuccess());
    EXPECT_FALSE(result.value());
}

// Test: 检查新持久化文件
TEST_F(StealthVerifierTest, HasNewPersistentFiles) {
    auto beforeResult = verifier.captureSystemState();
    ASSERT_TRUE(beforeResult.isSuccess());
    
    auto afterResult = verifier.captureSystemState();
    ASSERT_TRUE(afterResult.isSuccess());
    
    // 在没有创建新文件的情况下，应该返回 false
    auto result = verifier.hasNewPersistentFiles(
        beforeResult.value(),
        afterResult.value()
    );
    
    ASSERT_TRUE(result.isSuccess());
    EXPECT_FALSE(result.value());
}

// Test: 检查资源是否被清理
TEST_F(StealthVerifierTest, AreResourcesCleaned) {
    auto beforeResult = verifier.captureSystemState();
    ASSERT_TRUE(beforeResult.isSuccess());
    
    auto afterResult = verifier.captureSystemState();
    ASSERT_TRUE(afterResult.isSuccess());
    
    // 在没有泄漏资源的情况下，应该返回 true
    auto result = verifier.areResourcesCleaned(
        beforeResult.value(),
        afterResult.value()
    );
    
    ASSERT_TRUE(result.isSuccess());
    EXPECT_TRUE(result.value());
}

// Test: 验证完整的无痕操作
TEST_F(StealthVerifierTest, VerifyStealthOperation) {
    auto beforeResult = verifier.captureSystemState();
    ASSERT_TRUE(beforeResult.isSuccess());
    
    // 执行一些不会留下痕迹的操作
    // （这里只是等待，不做任何实际操作）
    usleep(100000);  // 等待 100ms
    
    auto afterResult = verifier.captureSystemState();
    ASSERT_TRUE(afterResult.isSuccess());
    
    // 验证操作是无痕的
    auto result = verifier.verifyStealthOperation(
        beforeResult.value(),
        afterResult.value()
    );
    
    ASSERT_TRUE(result.isSuccess());
    EXPECT_TRUE(result.value());
}

// Test: 系统状态快照的一致性
TEST_F(StealthVerifierTest, SystemSnapshotConsistency) {
    auto snapshot1 = verifier.captureSystemState();
    ASSERT_TRUE(snapshot1.isSuccess());
    
    auto snapshot2 = verifier.captureSystemState();
    ASSERT_TRUE(snapshot2.isSuccess());
    
    // 两个快照应该有相似的大小（允许一些变化）
    EXPECT_EQ(snapshot1.value().loadedModules.size(),
              snapshot2.value().loadedModules.size());
    
    // 文件描述符数量应该相同或接近
    int fdDiff = static_cast<int>(snapshot2.value().openFileDescriptors) -
                 static_cast<int>(snapshot1.value().openFileDescriptors);
    EXPECT_LE(std::abs(fdDiff), 5);  // 允许最多 5 个文件描述符的差异
}

// Test: 资源泄漏检测
TEST_F(StealthVerifierTest, ResourceLeakDetection) {
    auto beforeResult = verifier.captureSystemState();
    ASSERT_TRUE(beforeResult.isSuccess());
    
    // 打开一些文件描述符
    std::vector<int> fds;
    for (int i = 0; i < 5; ++i) {
        int fd = open("/dev/null", O_RDONLY);
        if (fd >= 0) {
            fds.push_back(fd);
        }
    }
    
    auto afterResult = verifier.captureSystemState();
    ASSERT_TRUE(afterResult.isSuccess());
    
    // 应该检测到文件描述符泄漏
    auto result = verifier.areResourcesCleaned(
        beforeResult.value(),
        afterResult.value()
    );
    
    ASSERT_TRUE(result.isSuccess());
    EXPECT_FALSE(result.value());
    
    // 清理
    for (int fd : fds) {
        close(fd);
    }
}

// Test: 模块加载检测
TEST_F(StealthVerifierTest, ModuleLoadingDetection) {
    auto beforeResult = verifier.captureSystemState();
    ASSERT_TRUE(beforeResult.isSuccess());
    
    // 创建一个修改后的快照，模拟新模块加载
    auto afterResult = verifier.captureSystemState();
    ASSERT_TRUE(afterResult.isSuccess());
    
    auto after = afterResult.value();
    after.loadedModules.insert("test_module");
    
    // 应该检测到新模块
    auto result = verifier.hasNewModulesLoaded(
        beforeResult.value(),
        after
    );
    
    ASSERT_TRUE(result.isSuccess());
    EXPECT_TRUE(result.value());
}

// Test: 文件创建检测
TEST_F(StealthVerifierTest, FileCreationDetection) {
    auto beforeResult = verifier.captureSystemState();
    ASSERT_TRUE(beforeResult.isSuccess());
    
    // 创建一个修改后的快照，模拟新文件创建
    auto afterResult = verifier.captureSystemState();
    ASSERT_TRUE(afterResult.isSuccess());
    
    auto after = afterResult.value();
    after.devFiles.insert("test_device");
    
    // 应该检测到新文件
    auto result = verifier.hasNewPersistentFiles(
        beforeResult.value(),
        after
    );
    
    ASSERT_TRUE(result.isSuccess());
    EXPECT_TRUE(result.value());
}
