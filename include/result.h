#ifndef USERSPACE_KERNEL_CALL_RESULT_H
#define USERSPACE_KERNEL_CALL_RESULT_H

#include <string>
#include <optional>
#include <stdexcept>

namespace ukc {

/**
 * 统一的错误处理类型
 * 所有可能失败的操作都返回 Result<T>
 * 
 * 使用示例：
 *   Result<int> res = someOperation();
 *   if (res.isSuccess()) {
 *       int value = res.value();
 *   } else {
 *       std::cerr << "Error: " << res.errorMessage() << std::endl;
 *   }
 */
template<typename T>
class Result {
public:
    /**
     * 创建成功结果
     */
    static Result<T> success(T value) {
        Result<T> result;
        result.success_ = true;
        result.value_ = std::move(value);
        return result;
    }

    /**
     * 创建错误结果
     */
    static Result<T> error(const std::string& message) {
        Result<T> result;
        result.success_ = false;
        result.errorMessage_ = message;
        return result;
    }

    /**
     * 检查是否成功
     */
    bool isSuccess() const {
        return success_;
    }

    /**
     * 检查是否失败
     */
    bool isError() const {
        return !success_;
    }

    /**
     * 获取值（仅在成功时调用）
     */
    T& value() {
        if (!success_) {
            throw std::runtime_error("Cannot get value from error result: " + errorMessage_);
        }
        return value_;
    }

    /**
     * 获取值（仅在成功时调用，const 版本）
     */
    const T& value() const {
        if (!success_) {
            throw std::runtime_error("Cannot get value from error result: " + errorMessage_);
        }
        return value_;
    }

    /**
     * 获取错误消息
     */
    const std::string& errorMessage() const {
        return errorMessage_;
    }

    /**
     * 移动值（仅在成功时调用）
     */
    T moveValue() {
        if (!success_) {
            throw std::runtime_error("Cannot move value from error result: " + errorMessage_);
        }
        return std::move(value_);
    }

private:
    bool success_ = false;
    T value_;
    std::string errorMessage_;
};

/**
 * Result<void> 特化版本
 */
template<>
class Result<void> {
public:
    /**
     * 创建成功结果
     */
    static Result<void> success() {
        Result<void> result;
        result.success_ = true;
        return result;
    }

    /**
     * 创建错误结果
     */
    static Result<void> error(const std::string& message) {
        Result<void> result;
        result.success_ = false;
        result.errorMessage_ = message;
        return result;
    }

    /**
     * 检查是否成功
     */
    bool isSuccess() const {
        return success_;
    }

    /**
     * 检查是否失败
     */
    bool isError() const {
        return !success_;
    }

    /**
     * 获取错误消息
     */
    const std::string& errorMessage() const {
        return errorMessage_;
    }

private:
    bool success_ = false;
    std::string errorMessage_;
};

} // namespace ukc

#endif // USERSPACE_KERNEL_CALL_RESULT_H
