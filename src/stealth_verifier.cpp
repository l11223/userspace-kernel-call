#include "stealth_verifier.h"
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

namespace ukc {

StealthVerifier::StealthVerifier() = default;

StealthVerifier::~StealthVerifier() = default;

Result<SystemSnapshot> StealthVerifier::captureSystemState() {
    SystemSnapshot snapshot;
    
    // 读取已加载的内核模块
    auto modulesResult = readLoadedModules();
    if (modulesResult.isError()) {
        return Result<SystemSnapshot>::error(
            "Failed to read loaded modules: " + modulesResult.errorMessage()
        );
    }
    snapshot.loadedModules = modulesResult.value();
    
    // 读取 /dev 中的文件
    auto devFilesResult = readDevFiles();
    if (devFilesResult.isError()) {
        return Result<SystemSnapshot>::error(
            "Failed to read /dev files: " + devFilesResult.errorMessage()
        );
    }
    snapshot.devFiles = devFilesResult.value();
    
    // 读取 /sys/module 中的文件
    auto sysModuleFilesResult = readSysModuleFiles();
    if (sysModuleFilesResult.isError()) {
        return Result<SystemSnapshot>::error(
            "Failed to read /sys/module files: " + sysModuleFilesResult.errorMessage()
        );
    }
    snapshot.sysModuleFiles = sysModuleFilesResult.value();
    
    // 获取打开的文件描述符数量
    auto fdCountResult = getOpenFileDescriptorCount();
    if (fdCountResult.isError()) {
        return Result<SystemSnapshot>::error(
            "Failed to get file descriptor count: " + fdCountResult.errorMessage()
        );
    }
    snapshot.openFileDescriptors = fdCountResult.value();
    
    // 获取内存使用量
    auto memoryResult = getProcessMemoryUsage();
    if (memoryResult.isError()) {
        return Result<SystemSnapshot>::error(
            "Failed to get memory usage: " + memoryResult.errorMessage()
        );
    }
    snapshot.allocatedMemory = memoryResult.value();
    
    return Result<SystemSnapshot>::success(std::move(snapshot));
}

Result<bool> StealthVerifier::hasNewModulesLoaded(
    const SystemSnapshot& before,
    const SystemSnapshot& after
) {
    // 检查是否有新的模块被加载
    for (const auto& module : after.loadedModules) {
        if (before.loadedModules.find(module) == before.loadedModules.end()) {
            return Result<bool>::success(true);
        }
    }
    
    return Result<bool>::success(false);
}

Result<bool> StealthVerifier::hasNewPersistentFiles(
    const SystemSnapshot& before,
    const SystemSnapshot& after
) {
    // 检查 /dev 中是否有新文件
    for (const auto& file : after.devFiles) {
        if (before.devFiles.find(file) == before.devFiles.end()) {
            return Result<bool>::success(true);
        }
    }
    
    // 检查 /sys/module 中是否有新文件
    for (const auto& file : after.sysModuleFiles) {
        if (before.sysModuleFiles.find(file) == before.sysModuleFiles.end()) {
            return Result<bool>::success(true);
        }
    }
    
    return Result<bool>::success(false);
}

Result<bool> StealthVerifier::areResourcesCleaned(
    const SystemSnapshot& before,
    const SystemSnapshot& after
) {
    // 检查文件描述符是否被正确关闭
    if (after.openFileDescriptors > before.openFileDescriptors) {
        return Result<bool>::success(false);
    }
    
    // 检查内存是否被正确释放
    // 允许一些小的差异（< 1MB）
    const size_t MEMORY_TOLERANCE = 1024 * 1024;
    if (after.allocatedMemory > before.allocatedMemory + MEMORY_TOLERANCE) {
        return Result<bool>::success(false);
    }
    
    return Result<bool>::success(true);
}

Result<bool> StealthVerifier::verifyStealthOperation(
    const SystemSnapshot& before,
    const SystemSnapshot& after
) {
    // 检查是否有新的模块被加载
    auto modulesResult = hasNewModulesLoaded(before, after);
    if (modulesResult.isError()) {
        return Result<bool>::error(modulesResult.errorMessage());
    }
    if (modulesResult.value()) {
        return Result<bool>::success(false);
    }
    
    // 检查是否有新的持久化文件
    auto filesResult = hasNewPersistentFiles(before, after);
    if (filesResult.isError()) {
        return Result<bool>::error(filesResult.errorMessage());
    }
    if (filesResult.value()) {
        return Result<bool>::success(false);
    }
    
    // 检查资源是否被正确清理
    auto resourcesResult = areResourcesCleaned(before, after);
    if (resourcesResult.isError()) {
        return Result<bool>::error(resourcesResult.errorMessage());
    }
    if (!resourcesResult.value()) {
        return Result<bool>::success(false);
    }
    
    return Result<bool>::success(true);
}

Result<std::set<std::string>> StealthVerifier::readLoadedModules() {
    std::set<std::string> modules;
    
    std::ifstream modulesFile("/proc/modules");
    if (!modulesFile.is_open()) {
        return Result<std::set<std::string>>::error(
            "Cannot open /proc/modules"
        );
    }
    
    std::string line;
    while (std::getline(modulesFile, line)) {
        if (line.empty()) continue;
        
        // 第一个字段是模块名称
        size_t spacePos = line.find(' ');
        if (spacePos != std::string::npos) {
            modules.insert(line.substr(0, spacePos));
        }
    }
    
    modulesFile.close();
    return Result<std::set<std::string>>::success(std::move(modules));
}

Result<std::set<std::string>> StealthVerifier::readDevFiles() {
    std::set<std::string> files;
    
    DIR* devDir = opendir("/dev");
    if (!devDir) {
        return Result<std::set<std::string>>::error(
            "Cannot open /dev directory"
        );
    }
    
    struct dirent* entry;
    while ((entry = readdir(devDir)) != nullptr) {
        if (entry->d_name[0] != '.') {
            files.insert(entry->d_name);
        }
    }
    
    closedir(devDir);
    return Result<std::set<std::string>>::success(std::move(files));
}

Result<std::set<std::string>> StealthVerifier::readSysModuleFiles() {
    std::set<std::string> files;
    
    DIR* sysModuleDir = opendir("/sys/module");
    if (!sysModuleDir) {
        return Result<std::set<std::string>>::error(
            "Cannot open /sys/module directory"
        );
    }
    
    struct dirent* entry;
    while ((entry = readdir(sysModuleDir)) != nullptr) {
        if (entry->d_name[0] != '.') {
            files.insert(entry->d_name);
        }
    }
    
    closedir(sysModuleDir);
    return Result<std::set<std::string>>::success(std::move(files));
}

Result<size_t> StealthVerifier::getOpenFileDescriptorCount() {
    pid_t pid = getpid();
    std::string fdPath = "/proc/" + std::to_string(pid) + "/fd";
    
    DIR* fdDir = opendir(fdPath.c_str());
    if (!fdDir) {
        return Result<size_t>::error(
            "Cannot open " + fdPath
        );
    }
    
    size_t count = 0;
    struct dirent* entry;
    while ((entry = readdir(fdDir)) != nullptr) {
        if (entry->d_name[0] != '.') {
            count++;
        }
    }
    
    closedir(fdDir);
    return Result<size_t>::success(count);
}

Result<size_t> StealthVerifier::getProcessMemoryUsage() {
    pid_t pid = getpid();
    std::string statusPath = "/proc/" + std::to_string(pid) + "/status";
    
    std::ifstream statusFile(statusPath);
    if (!statusFile.is_open()) {
        return Result<size_t>::error(
            "Cannot open " + statusPath
        );
    }
    
    std::string line;
    while (std::getline(statusFile, line)) {
        if (line.find("VmRSS:") == 0) {
            // 格式: VmRSS:     1234 kB
            std::istringstream iss(line);
            std::string label;
            size_t memory;
            iss >> label >> memory;
            
            statusFile.close();
            return Result<size_t>::success(memory * 1024);  // 转换为字节
        }
    }
    
    statusFile.close();
    return Result<size_t>::error("VmRSS not found in /proc/status");
}

} // namespace ukc
