#ifndef USERSPACE_KERNEL_CALL_ARM64_ASSEMBLY_BRIDGE_H
#define USERSPACE_KERNEL_CALL_ARM64_ASSEMBLY_BRIDGE_H

#include <cstdint>
#include <cstddef>

namespace ukc {
namespace arm64 {

/**
 * ARM64 汇编桥接
 * 实现用户态到内核的函数调用
 */

/**
 * 调用内核函数
 * 
 * 遵循 ARM64 AAPCS 调用约定：
 * - x0-x5: 前 6 个参数
 * - x30 (LR): 返回地址
 * - x0: 返回值
 * 
 * @param kernel_func_addr 内核函数地址
 * @param arg0-arg5 函数参数
 * @return 内核函数的返回值
 */
extern "C" uint64_t kernel_call_bridge(
    uintptr_t kernel_func_addr,
    uint64_t arg0, uint64_t arg1, uint64_t arg2,
    uint64_t arg3, uint64_t arg4, uint64_t arg5
);

/**
 * 生成跳转指令
 * 
 * @param from 跳转源地址
 * @param to 跳转目标地址
 * @param out 输出缓冲区
 * @param out_size 输出大小
 */
void generate_jump_instruction(
    uint64_t from,
    uint64_t to,
    uint8_t* out,
    size_t* out_size
);

/**
 * 生成跳板代码
 * 
 * @param orig_bytes 原始指令字节
 * @param orig_size 原始指令大小
 * @param return_addr 返回地址
 * @param out 输出缓冲区
 * @param out_size 输出大小
 */
void generate_trampoline(
    const uint8_t* orig_bytes,
    size_t orig_size,
    uint64_t return_addr,
    uint8_t* out,
    size_t* out_size
);

/**
 * 计算指令大小
 * 
 * @param code 代码指针（可以为 nullptr）
 * @param min_size 最小大小
 * @return 计算出的指令大小
 */
size_t calculate_instruction_size(
    const uint8_t* code,
    size_t min_size
);

} // namespace arm64
} // namespace ukc

#endif // USERSPACE_KERNEL_CALL_ARM64_ASSEMBLY_BRIDGE_H
