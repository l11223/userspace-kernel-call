#include "arm64_assembly_bridge.h"
#include <cstring>

namespace ukc {
namespace arm64 {

/**
 * ARM64 内核调用桥接
 * 
 * 使用内联汇编实现用户态到内核的函数调用
 * 遵循 ARM64 AAPCS 调用约定
 */
extern "C" uint64_t kernel_call_bridge(
    uintptr_t kernel_func_addr,
    uint64_t arg0, uint64_t arg1, uint64_t arg2,
    uint64_t arg3, uint64_t arg4, uint64_t arg5
) {
    uint64_t result;
    
    __asm__ __volatile__(
        // 保存寄存器
        "stp x29, x30, [sp, #-16]!\n"
        "mov x29, sp\n"
        
        // 加载参数到寄存器（ARM64 AAPCS）
        "mov x0, %1\n"   // arg0 -> x0
        "mov x1, %2\n"   // arg1 -> x1
        "mov x2, %3\n"   // arg2 -> x2
        "mov x3, %4\n"   // arg3 -> x3
        "mov x4, %5\n"   // arg4 -> x4
        "mov x5, %6\n"   // arg5 -> x5
        
        // 调用内核函数
        // 使用 blr 指令跳转到内核函数地址
        "blr %7\n"       // 跳转到内核函数地址
        
        // 保存返回值
        "mov %0, x0\n"
        
        // 恢复寄存器
        "ldp x29, x30, [sp], #16\n"
        
        : "=r"(result)                          // 输出：result
        : "r"(arg0), "r"(arg1), "r"(arg2),     // 输入：参数
          "r"(arg3), "r"(arg4), "r"(arg5),
          "r"(kernel_func_addr)                 // 输入：内核函数地址
        : "x0", "x1", "x2", "x3", "x4", "x5",  // 被破坏的寄存器
          "memory"                              // 内存被修改
    );
    
    return result;
}

void generate_jump_instruction(
    uint64_t from,
    uint64_t to,
    uint8_t* out,
    size_t* out_size
) {
    int64_t offset = to - from;
    
    if (offset >= -0x8000000 && offset < 0x8000000) {
        // 使用 B 指令（相对跳转，±128MB）
        // B 指令格式: 0001 01xx xxxx xxxx xxxx xxxx xxxx xxxx
        // 其中 x 是 26 位有符号偏移（以 4 字节为单位）
        uint32_t insn = 0x14000000 | ((offset >> 2) & 0x3FFFFFF);
        memcpy(out, &insn, 4);
        *out_size = 4;
    } else {
        // 使用 LDR + BR（远跳转）
        // LDR X16, #8  (加载 8 字节后的地址)
        // BR X16       (跳转到 X16)
        // .quad <address>
        
        uint32_t ldr = 0x58000050;  // LDR X16, #8
        uint32_t br = 0xD61F0200;   // BR X16
        
        memcpy(out, &ldr, 4);
        memcpy(out + 4, &br, 4);
        memcpy(out + 8, &to, 8);
        *out_size = 16;
    }
}

void generate_trampoline(
    const uint8_t* orig_bytes,
    size_t orig_size,
    uint64_t return_addr,
    uint8_t* out,
    size_t* out_size
) {
    // 跳板 = 原始指令 + 跳转回原函数
    memcpy(out, orig_bytes, orig_size);
    
    size_t jump_size;
    generate_jump_instruction(
        0,  // 相对地址（从跳板开始）
        return_addr,
        out + orig_size,
        &jump_size
    );
    
    *out_size = orig_size + jump_size;
}

size_t calculate_instruction_size(
    const uint8_t* code,
    size_t min_size
) {
    // ARM64 指令固定 4 字节
    // 我们需要至少 16 字节来放置远跳转（LDR + BR + 地址）
    size_t size = 0;
    while (size < min_size || size < 16) {
        size += 4;
    }
    return size;
}

} // namespace arm64
} // namespace ukc
