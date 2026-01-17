#ifndef USERSPACE_KERNEL_CALL_MAGISK_INTERFACE_H
#define USERSPACE_KERNEL_CALL_MAGISK_INTERFACE_H

#include <cstdint>
#include <cstddef>

namespace ukc {
namespace magisk {

/**
 * Magisk 接口封装
 * 用于在安卓15上安全地修改内核内存
 */

/**
 * 检查是否运行在 Magisk 环境中
 * @return true 如果 Magisk 可用
 */
bool is_magisk_available();

/**
 * 通过 Magisk 接口设置内存为可读写
 * @param addr 内存地址
 * @param size 大小
 * @return 0 成功，-1 失败
 */
int magisk_set_memory_rw(void* addr, size_t size);

/**
 * 通过 Magisk 接口设置内存为只读
 * @param addr 内存地址
 * @param size 大小
 * @return 0 成功，-1 失败
 */
int magisk_set_memory_ro(void* addr, size_t size);

/**
 * 通过 Magisk 接口读取内核内存
 * @param addr 内核地址
 * @param buf 输出缓冲区
 * @param size 大小
 * @return 实际读取的字节数，-1 表示失败
 */
ssize_t magisk_read_kernel_memory(uintptr_t addr, void* buf, size_t size);

/**
 * 通过 Magisk 接口写入内核内存
 * @param addr 内核地址
 * @param buf 数据缓冲区
 * @param size 大小
 * @return 实际写入的字节数，-1 表示失败
 */
ssize_t magisk_write_kernel_memory(uintptr_t addr, const void* buf, size_t size);

/**
 * 通过 Magisk 接口查找内核符号
 * @param name 符号名称
 * @return 符号地址，0 表示未找到
 */
uintptr_t magisk_kallsyms_lookup_name(const char* name);

/**
 * 安全的内存修改包装器
 * 自动处理内存保护和恢复
 */
class SafeMemoryModifier {
public:
    /**
     * 修改内核内存（自动处理保护）
     * @param addr 地址
     * @param data 数据
     * @param size 大小
     * @return 0 成功，-1 失败
     */
    static int modify_kernel_memory(uintptr_t addr, const void* data, size_t size);

    /**
     * 读取内核内存（自动处理保护）
     * @param addr 地址
     * @param buf 输出缓冲区
     * @param size 大小
     * @return 实际读取的字节数，-1 表示失败
     */
    static ssize_t read_kernel_memory(uintptr_t addr, void* buf, size_t size);
};

} // namespace magisk
} // namespace ukc

#endif // USERSPACE_KERNEL_CALL_MAGISK_INTERFACE_H
