#include "userspace_kernel_call.h"
#include <iostream>
#include <iomanip>

using namespace ukc;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "用法: " << argv[0] << " <进程名>" << std::endl;
        std::cerr << "示例: " << argv[0] << " target_app" << std::endl;
        return 1;
    }
    
    std::string processName = argv[1];
    
    // 创建系统对象
    UserspaceKernelCall ukc;
    
    // 初始化
    std::cout << "正在初始化系统..." << std::endl;
    auto initResult = ukc.initialize();
    if (initResult.isError()) {
        std::cerr << "❌ 初始化失败: " << initResult.errorMessage() << std::endl;
        std::cerr << "提示: 需要 Root 权限，请用 sudo 运行" << std::endl;
        return 1;
    }
    std::cout << "✓ 系统初始化成功" << std::endl;
    
    // 查找进程
    std::cout << "\n正在查找进程: " << processName << std::endl;
    auto pidResult = ukc.findProcessByName(processName);
    if (pidResult.isError()) {
        std::cerr << "❌ 进程未找到: " << pidResult.errorMessage() << std::endl;
        return 1;
    }
    
    pid_t targetPid = pidResult.value();
    std::cout << "✓ 找到进程 PID: " << targetPid << std::endl;
    
    // 获取内存映射
    std::cout << "\n正在获取进程内存映射..." << std::endl;
    auto mapsResult = ukc.getProcessMemoryMaps(targetPid);
    if (mapsResult.isError()) {
        std::cerr << "❌ 获取内存映射失败: " << mapsResult.errorMessage() << std::endl;
        return 1;
    }
    
    const auto& regions = mapsResult.value();
    std::cout << "✓ 找到 " << regions.size() << " 个内存区域" << std::endl;
    
    // 显示前几个内存区域
    std::cout << "\n内存区域列表:" << std::endl;
    for (size_t i = 0; i < std::min(size_t(5), regions.size()); ++i) {
        const auto& region = regions[i];
        std::cout << "  [" << i << "] 0x" << std::hex << region.start 
                  << " - 0x" << region.end << std::dec
                  << " (" << (region.end - region.start) / 1024 << " KB)"
                  << " " << region.permissions;
        if (!region.path.empty()) {
            std::cout << " " << region.path;
        }
        std::cout << std::endl;
    }
    
    // 从第一个可读区域读取内存
    std::cout << "\n正在读取内存..." << std::endl;
    uintptr_t readAddr = regions[0].start;
    size_t readSize = 256;
    
    std::cout << "读取地址: 0x" << std::hex << readAddr << std::dec << std::endl;
    std::cout << "读取大小: " << readSize << " 字节" << std::endl;
    
    auto readResult = ukc.readMemory(targetPid, readAddr, readSize);
    if (readResult.isError()) {
        std::cerr << "❌ 读取失败: " << readResult.errorMessage() << std::endl;
        return 1;
    }
    
    const auto& data = readResult.value();
    std::cout << "✓ 成功读取 " << data.size() << " 字节" << std::endl;
    
    // 显示读取的数据（十六进制）
    std::cout << "\n读取的数据 (前 64 字节):" << std::endl;
    for (size_t i = 0; i < std::min(size_t(64), data.size()); ++i) {
        if (i % 16 == 0) {
            std::cout << "  0x" << std::hex << std::setw(8) << std::setfill('0') 
                      << (readAddr + i) << ": ";
        }
        std::cout << std::hex << std::setw(2) << std::setfill('0') 
                  << (int)data[i] << " ";
        if ((i + 1) % 16 == 0) {
            std::cout << std::endl;
        }
    }
    std::cout << std::dec << std::endl;
    
    std::cout << "\n✓ 操作完成！" << std::endl;
    return 0;
}
