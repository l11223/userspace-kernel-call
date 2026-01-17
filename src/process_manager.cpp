#include "process_manager.h"
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <dirent.h>
#include <cstdlib>

namespace ukc {

ProcessManager::ProcessManager() = default;

ProcessManager::~ProcessManager() = default;

Result<pid_t> ProcessManager::findProcessByName(const std::string& processName) {
    // 遍历 /proc 目录查找进程
    DIR* procDir = opendir("/proc");
    if (!procDir) {
        return Result<pid_t>::error("Cannot open /proc directory");
    }
    
    struct dirent* entry;
    while ((entry = readdir(procDir)) != nullptr) {
        // 检查是否是数字目录（进程 ID）
        if (entry->d_type != DT_DIR) continue;
        
        char* endptr;
        pid_t pid = strtol(entry->d_name, &endptr, 10);
        if (*endptr != '\0') continue;  // 不是纯数字
        
        // 读取进程的 cmdline
        std::string cmdlinePath = "/proc/" + std::to_string(pid) + "/cmdline";
        std::ifstream cmdlineFile(cmdlinePath);
        if (!cmdlineFile.is_open()) continue;
        
        std::string cmdline;
        std::getline(cmdlineFile, cmdline);
        cmdlineFile.close();
        
        // 替换 null 字符为空格（cmdline 中的参数用 null 分隔）
        for (char& c : cmdline) {
            if (c == '\0') c = ' ';
        }
        
        // 检查进程名称是否匹配
        if (cmdline.find(processName) != std::string::npos) {
            closedir(procDir);
            return Result<pid_t>::success(pid);
        }
    }
    
    closedir(procDir);
    return Result<pid_t>::error("Process '" + processName + "' not found");
}

bool ProcessManager::isProcessAlive(pid_t pid) const {
    // 检查 /proc/pid 是否存在
    std::string procPath = "/proc/" + std::to_string(pid);
    struct stat buffer;
    return stat(procPath.c_str(), &buffer) == 0;
}

Result<std::vector<MemoryRegion>> ProcessManager::getMemoryMaps(pid_t pid) {
    std::string mapsPath = "/proc/" + std::to_string(pid) + "/maps";
    std::ifstream mapsFile(mapsPath);
    
    if (!mapsFile.is_open()) {
        return Result<std::vector<MemoryRegion>>::error(
            "Cannot open " + mapsPath
        );
    }
    
    std::stringstream buffer;
    buffer << mapsFile.rdbuf();
    
    return parseMemoryMaps(buffer.str());
}

bool ProcessManager::isValidAddress(pid_t pid, uintptr_t address) {
    auto mapsResult = getMemoryMaps(pid);
    if (mapsResult.isError()) {
        return false;
    }
    
    const auto& regions = mapsResult.value();
    for (const auto& region : regions) {
        if (address >= region.start && address < region.end) {
            return true;
        }
    }
    
    return false;
}

Result<std::vector<MemoryRegion>> ProcessManager::parseMemoryMaps(
    const std::string& mapsContent
) {
    std::vector<MemoryRegion> regions;
    std::istringstream stream(mapsContent);
    std::string line;
    
    while (std::getline(stream, line)) {
        if (line.empty()) continue;
        
        // 解析 /proc/pid/maps 格式
        // 例如: 7f7d8c000000-7f7d8c021000 r--p 00000000 08:01 1234567 /path/to/file
        
        MemoryRegion region;
        std::istringstream lineStream(line);
        
        std::string addressRange;
        lineStream >> addressRange;
        
        // 解析地址范围
        size_t dashPos = addressRange.find('-');
        if (dashPos == std::string::npos) {
            continue;
        }
        
        try {
            region.start = std::stoull(addressRange.substr(0, dashPos), nullptr, 16);
            region.end = std::stoull(addressRange.substr(dashPos + 1), nullptr, 16);
        } catch (...) {
            continue;
        }
        
        // 解析权限
        lineStream >> region.permissions;
        
        // 跳过偏移和设备号
        std::string offset, device;
        lineStream >> offset >> device;
        
        // 跳过 inode
        std::string inode;
        lineStream >> inode;
        
        // 解析路径（可选）
        if (lineStream >> region.path) {
            // 路径已读取
        }
        
        regions.push_back(region);
    }
    
    return Result<std::vector<MemoryRegion>>::success(std::move(regions));
}

} // namespace ukc
