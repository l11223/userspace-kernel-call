#ifndef USERSPACE_KERNEL_CALL_SKROOT_INTERFACE_H
#define USERSPACE_KERNEL_CALL_SKROOT_INTERFACE_H

#include <cstdint>
#include <cstddef>
#include "result.h"

namespace ukc {
namespace skroot {

/**
 * SKRoot 集成接口
 * 
 * SKRoot 是一个用户态 Root 解决方案，无需加载内核模块
 * 支持 Android 15 + Linux 6.6.56 + ARM64-v8a
 */

/**
 * SKRoot 初始化
 * 
 * @return 成功返回 true，失败返回 false
 */
Result<bool> initialize_skroot();

/**
 * 检查 SKRoot 是否可用
 * 
 * @return 可用返回 true
 */
Result<bool> is_skroot_available();

/**
 * 通过 SKRoot 执行内核函数
 * 
 * @param kernel_func_addr 内核函数地址
 * @param args 函数参数数组（最多 6 个）
 * @param arg_count 参数个数
 * @return 内核函数返回值
 */
Result<uint64_t> call_kernel_function(
    uintptr_t kernel_func_addr,
    const uint64_t* args,
    size_t arg_count
);

/**
 * 通过 SKRoot 读取内核内存
 * 
 * @param kernel_addr 内核地址
 * @param buffer 输出缓冲区
 * @param size 读取大小
 * @return 实际读取的字节数
 */
Result<size_t> read_kernel_memory(
    uintptr_t kernel_addr,
    uint8_t* buffer,
    size_t size
);

/**
 * 通过 SKRoot 写入内核内存
 * 
 * @param kernel_addr 内核地址
 * @param data 数据缓冲区
 * @param size 写入大小
 * @return 实际写入的字节数
 */
Result<size_t> write_kernel_memory(
    uintptr_t kernel_addr,
    const uint8_t* data,
    size_t size
);

/**
 * 获取 SKRoot 版本信息
 * 
 * @return 版本字符串
 */
Result<const char*> get_skroot_version();

/**
 * 清理 SKRoot 资源
 */
void cleanup_skroot();

} // namespace skroot
} // namespace ukc

#endif // USERSPACE_KERNEL_CALL_SKROOT_INTERFACE_H
