#include <gtest/gtest.h>
#include "result.h"

using namespace ukc;

class ResultTest : public ::testing::Test {
};

// 测试成功结果
TEST_F(ResultTest, SuccessResult) {
    Result<int> result = Result<int>::success(42);
    
    EXPECT_TRUE(result.isSuccess());
    EXPECT_FALSE(result.isError());
    EXPECT_EQ(result.value(), 42);
}

// 测试错误结果
TEST_F(ResultTest, ErrorResult) {
    Result<int> result = Result<int>::error("Test error");
    
    EXPECT_FALSE(result.isSuccess());
    EXPECT_TRUE(result.isError());
    EXPECT_EQ(result.errorMessage(), "Test error");
}

// 测试从错误结果获取值会抛出异常
TEST_F(ResultTest, GetValueFromErrorThrows) {
    Result<int> result = Result<int>::error("Test error");
    
    EXPECT_THROW(result.value(), std::runtime_error);
}

// 测试 Result<void> 成功
TEST_F(ResultTest, VoidResultSuccess) {
    Result<void> result = Result<void>::success();
    
    EXPECT_TRUE(result.isSuccess());
    EXPECT_FALSE(result.isError());
}

// 测试 Result<void> 错误
TEST_F(ResultTest, VoidResultError) {
    Result<void> result = Result<void>::error("Test error");
    
    EXPECT_FALSE(result.isSuccess());
    EXPECT_TRUE(result.isError());
    EXPECT_EQ(result.errorMessage(), "Test error");
}

// 测试移动值
TEST_F(ResultTest, MoveValue) {
    Result<std::string> result = Result<std::string>::success("Hello");
    
    std::string value = result.moveValue();
    EXPECT_EQ(value, "Hello");
}
