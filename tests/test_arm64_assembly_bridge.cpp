#include <gtest/gtest.h>
#include "arm64_assembly_bridge.h"

using namespace ukc::arm64;

class ARM64AssemblyBridgeTest : public ::testing::Test {
};

// 测试跳转指令生成 - 短距离
TEST_F(ARM64AssemblyBridgeTest, GenerateJumpInstructionShort) {
    uint8_t out[16];
    size_t out_size;
    
    // 生成短距离跳转（在 ±128MB 范围内）
    uint64_t from = 0x1000;
    uint64_t to = 0x2000;
    
    generate_jump_instruction(from, to, out, &out_size);
    
    // 应该使用 B 指令（4 字节）
    EXPECT_EQ(out_size, 4);
    
    // 验证指令格式（B 指令）
    uint32_t insn = *(uint32_t*)out;
    EXPECT_EQ((insn >> 26) & 0x3F, 0x05);  // B 指令的操作码
}

// 测试跳转指令生成 - 长距离
TEST_F(ARM64AssemblyBridgeTest, GenerateJumpInstructionLong) {
    uint8_t out[16];
    size_t out_size;
    
    // 生成长距离跳转（超过 ±128MB）
    uint64_t from = 0xFFFFFF8000000000UL;
    uint64_t to = 0xFFFFFF8100000000UL;
    
    generate_jump_instruction(from, to, out, &out_size);
    
    // 应该使用 LDR + BR（16 字节）
    EXPECT_EQ(out_size, 16);
    
    // 验证指令格式
    uint32_t ldr = *(uint32_t*)out;
    uint32_t br = *(uint32_t*)(out + 4);
    uint64_t addr = *(uint64_t*)(out + 8);
    
    EXPECT_EQ(ldr, 0x58000050);  // LDR X16, #8
    EXPECT_EQ(br, 0xD61F0200);   // BR X16
    EXPECT_EQ(addr, to);         // 目标地址
}

// 测试跳板生成
TEST_F(ARM64AssemblyBridgeTest, GenerateTrampoline) {
    uint8_t orig_bytes[] = {0x1F, 0x20, 0x03, 0xD5};  // NOP 指令
    uint8_t out[32];
    size_t out_size;
    
    uint64_t return_addr = 0x1000;
    
    generate_trampoline(orig_bytes, sizeof(orig_bytes), return_addr, out, &out_size);
    
    // 跳板应该包含原始指令 + 跳转指令
    EXPECT_GT(out_size, sizeof(orig_bytes));
    
    // 验证原始指令被复制
    EXPECT_EQ(memcmp(out, orig_bytes, sizeof(orig_bytes)), 0);
}

// 测试指令大小计算
TEST_F(ARM64AssemblyBridgeTest, CalculateInstructionSize) {
    // 最小大小为 0
    size_t size1 = calculate_instruction_size(nullptr, 0);
    EXPECT_GE(size1, 16);  // 至少 16 字节
    
    // 最小大小为 8
    size_t size2 = calculate_instruction_size(nullptr, 8);
    EXPECT_GE(size2, 16);  // 至少 16 字节
    
    // 最小大小为 20
    size_t size3 = calculate_instruction_size(nullptr, 20);
    EXPECT_GE(size3, 20);  // 至少 20 字节
    
    // 所有大小都应该是 4 的倍数（ARM64 指令大小）
    EXPECT_EQ(size1 % 4, 0);
    EXPECT_EQ(size2 % 4, 0);
    EXPECT_EQ(size3 % 4, 0);
}

// 测试跳转指令的对称性
TEST_F(ARM64AssemblyBridgeTest, JumpInstructionSymmetry) {
    uint8_t out1[16], out2[16];
    size_t size1, size2;
    
    uint64_t from = 0x1000;
    uint64_t to = 0x2000;
    
    // 生成从 from 到 to 的跳转
    generate_jump_instruction(from, to, out1, &size1);
    
    // 生成从 to 到 from 的跳转
    generate_jump_instruction(to, from, out2, &size2);
    
    // 两个跳转的大小应该相同
    EXPECT_EQ(size1, size2);
}

// 测试零偏移跳转
TEST_F(ARM64AssemblyBridgeTest, ZeroOffsetJump) {
    uint8_t out[16];
    size_t out_size;
    
    uint64_t addr = 0x1000;
    
    // 生成零偏移跳转（跳转到自己）
    generate_jump_instruction(addr, addr, out, &out_size);
    
    // 应该生成有效的指令
    EXPECT_GT(out_size, 0);
    EXPECT_LE(out_size, 16);
}

// 测试负偏移跳转
TEST_F(ARM64AssemblyBridgeTest, NegativeOffsetJump) {
    uint8_t out[16];
    size_t out_size;
    
    uint64_t from = 0x2000;
    uint64_t to = 0x1000;
    
    // 生成负偏移跳转
    generate_jump_instruction(from, to, out, &out_size);
    
    // 应该生成有效的指令
    EXPECT_GT(out_size, 0);
    EXPECT_LE(out_size, 16);
}
