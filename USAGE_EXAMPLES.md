# Usage Examples: Userspace Kernel Call

This document provides practical examples of how to use the Userspace Kernel Call library.

## Basic Setup

### Initialize the System

```cpp
#include "userspace_kernel_call.h"

using namespace ukc;

int main() {
    // Create the main system object
    UserspaceKernelCall ukc;
    
    // Initialize all subsystems
    auto result = ukc.initialize();
    if (result.isError()) {
        std::cerr << "Initialization failed: " << result.errorMessage() << std::endl;
        return 1;
    }
    
    // System is now ready to use
    return 0;
}
```

## Example 1: Read Process Memory

```cpp
#include "userspace_kernel_call.h"
#include <iostream>

using namespace ukc;

int main() {
    UserspaceKernelCall ukc;
    
    // Initialize
    auto initResult = ukc.initialize();
    if (initResult.isError()) {
        std::cerr << "Initialization failed" << std::endl;
        return 1;
    }
    
    // Find target process
    auto pidResult = ukc.findProcessByName("target_app");
    if (pidResult.isError()) {
        std::cerr << "Process not found: " << pidResult.errorMessage() << std::endl;
        return 1;
    }
    
    pid_t targetPid = pidResult.value();
    std::cout << "Found process: " << targetPid << std::endl;
    
    // Get process memory maps
    auto mapsResult = ukc.getProcessMemoryMaps(targetPid);
    if (mapsResult.isError()) {
        std::cerr << "Failed to get memory maps" << std::endl;
        return 1;
    }
    
    // Read memory from first region
    const auto& regions = mapsResult.value();
    if (regions.empty()) {
        std::cerr << "No memory regions found" << std::endl;
        return 1;
    }
    
    uintptr_t address = regions[0].start;
    size_t size = 256;
    
    auto readResult = ukc.readMemory(targetPid, address, size);
    if (readResult.isError()) {
        std::cerr << "Read failed: " << readResult.errorMessage() << std::endl;
        return 1;
    }
    
    // Process the read data
    const auto& data = readResult.value();
    std::cout << "Read " << data.size() << " bytes from 0x" 
              << std::hex << address << std::endl;
    
    // Print first 16 bytes as hex
    for (size_t i = 0; i < std::min(size_t(16), data.size()); ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') 
                  << (int)data[i] << " ";
    }
    std::cout << std::endl;
    
    return 0;
}
```

## Example 2: Write Process Memory

```cpp
#include "userspace_kernel_call.h"
#include <iostream>
#include <vector>

using namespace ukc;

int main() {
    UserspaceKernelCall ukc;
    
    // Initialize
    auto initResult = ukc.initialize();
    if (initResult.isError()) {
        std::cerr << "Initialization failed" << std::endl;
        return 1;
    }
    
    // Find target process
    auto pidResult = ukc.findProcessByName("target_app");
    if (pidResult.isError()) {
        std::cerr << "Process not found" << std::endl;
        return 1;
    }
    
    pid_t targetPid = pidResult.value();
    
    // Get process memory maps
    auto mapsResult = ukc.getProcessMemoryMaps(targetPid);
    if (mapsResult.isError()) {
        std::cerr << "Failed to get memory maps" << std::endl;
        return 1;
    }
    
    // Prepare data to write
    std::vector<uint8_t> data = {0x90, 0x90, 0x90, 0x90};  // NOP instructions
    
    uintptr_t address = mapsResult.value()[0].start;
    
    // Write memory
    auto writeResult = ukc.writeMemory(targetPid, address, data);
    if (writeResult.isError()) {
        std::cerr << "Write failed: " << writeResult.errorMessage() << std::endl;
        return 1;
    }
    
    std::cout << "Wrote " << writeResult.value() << " bytes to 0x" 
              << std::hex << address << std::endl;
    
    return 0;
}
```

## Example 3: Batch Memory Operations

```cpp
#include "userspace_kernel_call.h"
#include <iostream>
#include <vector>

using namespace ukc;

int main() {
    UserspaceKernelCall ukc;
    
    // Initialize
    auto initResult = ukc.initialize();
    if (initResult.isError()) {
        std::cerr << "Initialization failed" << std::endl;
        return 1;
    }
    
    // Find target process
    auto pidResult = ukc.findProcessByName("target_app");
    if (pidResult.isError()) {
        std::cerr << "Process not found" << std::endl;
        return 1;
    }
    
    pid_t targetPid = pidResult.value();
    
    // Get process memory maps
    auto mapsResult = ukc.getProcessMemoryMaps(targetPid);
    if (mapsResult.isError()) {
        std::cerr << "Failed to get memory maps" << std::endl;
        return 1;
    }
    
    uintptr_t baseAddr = mapsResult.value()[0].start;
    
    // Create batch operations
    std::vector<MemoryOperation> operations;
    
    // Operation 1: Read 256 bytes
    MemoryOperation readOp;
    readOp.type = OperationType::Read;
    readOp.address = baseAddr;
    readOp.size = 256;
    operations.push_back(readOp);
    
    // Operation 2: Write 4 bytes
    MemoryOperation writeOp;
    writeOp.type = OperationType::Write;
    writeOp.address = baseAddr + 256;
    writeOp.data = {0x90, 0x90, 0x90, 0x90};
    operations.push_back(writeOp);
    
    // Operation 3: Read 256 bytes again
    MemoryOperation readOp2;
    readOp2.type = OperationType::Read;
    readOp2.address = baseAddr + 512;
    readOp2.size = 256;
    operations.push_back(readOp2);
    
    // Execute batch operations
    auto batchResult = ukc.batchOperations(targetPid, operations);
    if (batchResult.isError()) {
        std::cerr << "Batch operations failed: " << batchResult.errorMessage() << std::endl;
        return 1;
    }
    
    // Process results
    for (size_t i = 0; i < operations.size(); ++i) {
        const auto& op = operations[i];
        if (op.success) {
            if (op.type == OperationType::Read) {
                std::cout << "Read operation " << i << " succeeded: " 
                          << op.result.size() << " bytes" << std::endl;
            } else {
                std::cout << "Write operation " << i << " succeeded" << std::endl;
            }
        } else {
            std::cout << "Operation " << i << " failed: " << op.errorMessage() << std::endl;
        }
    }
    
    return 0;
}
```

## Example 4: Error Handling

```cpp
#include "userspace_kernel_call.h"
#include <iostream>

using namespace ukc;

int main() {
    UserspaceKernelCall ukc;
    
    // Initialize with error handling
    auto initResult = ukc.initialize();
    if (initResult.isError()) {
        std::cerr << "Initialization failed: " << initResult.errorMessage() << std::endl;
        std::cerr << "Possible causes:" << std::endl;
        std::cerr << "  - Not running as root" << std::endl;
        std::cerr << "  - Kernel memory mapping unavailable" << std::endl;
        return 1;
    }
    
    // Try to find a process
    auto pidResult = ukc.findProcessByName("nonexistent_process");
    if (pidResult.isError()) {
        std::cerr << "Process not found: " << pidResult.errorMessage() << std::endl;
        // This is expected, continue with a known process
    }
    
    // Try to read from invalid address
    auto readResult = ukc.readMemory(1, 0xFFFFFFFFFFFFFFFFUL, 100);
    if (readResult.isError()) {
        std::cerr << "Read failed (expected): " << readResult.errorMessage() << std::endl;
    }
    
    return 0;
}
```

## Example 5: Performance Monitoring

```cpp
#include "userspace_kernel_call.h"
#include "performance_monitor.h"
#include <iostream>
#include <chrono>

using namespace ukc;

int main() {
    UserspaceKernelCall ukc;
    PerformanceMonitor monitor;
    
    // Initialize
    auto initResult = ukc.initialize();
    if (initResult.isError()) {
        std::cerr << "Initialization failed" << std::endl;
        return 1;
    }
    
    // Find target process
    auto pidResult = ukc.findProcessByName("target_app");
    if (pidResult.isError()) {
        std::cerr << "Process not found" << std::endl;
        return 1;
    }
    
    pid_t targetPid = pidResult.value();
    
    // Get memory maps
    auto mapsResult = ukc.getProcessMemoryMaps(targetPid);
    if (mapsResult.isError()) {
        std::cerr << "Failed to get memory maps" << std::endl;
        return 1;
    }
    
    uintptr_t address = mapsResult.value()[0].start;
    
    // Perform multiple read operations and measure performance
    for (int i = 0; i < 10; ++i) {
        monitor.startTimer("memory_read");
        
        auto readResult = ukc.readMemory(targetPid, address, 1024);
        
        monitor.stopTimer("memory_read");
        
        if (readResult.isError()) {
            std::cerr << "Read failed" << std::endl;
            return 1;
        }
    }
    
    // Get performance statistics
    auto statsResult = monitor.getStats("memory_read");
    if (statsResult.isSuccess()) {
        const auto& stats = statsResult.value();
        std::cout << stats.toString();
        
        // Check if performance meets requirements
        auto requirementResult = monitor.meetsPerformanceRequirement(
            "memory_read",
            std::chrono::milliseconds(100)
        );
        
        if (requirementResult.isSuccess()) {
            if (requirementResult.value()) {
                std::cout << "Performance requirement met!" << std::endl;
            } else {
                std::cout << "Performance requirement NOT met" << std::endl;
            }
        }
    }
    
    return 0;
}
```

## Example 6: Stealth Verification

```cpp
#include "userspace_kernel_call.h"
#include "stealth_verifier.h"
#include <iostream>

using namespace ukc;

int main() {
    UserspaceKernelCall ukc;
    StealthVerifier verifier;
    
    // Initialize
    auto initResult = ukc.initialize();
    if (initResult.isError()) {
        std::cerr << "Initialization failed" << std::endl;
        return 1;
    }
    
    // Capture system state before operations
    auto beforeResult = verifier.captureSystemState();
    if (beforeResult.isError()) {
        std::cerr << "Failed to capture system state" << std::endl;
        return 1;
    }
    
    const auto& beforeState = beforeResult.value();
    std::cout << "Captured system state before operations" << std::endl;
    std::cout << "  Loaded modules: " << beforeState.loadedModules.size() << std::endl;
    std::cout << "  /dev files: " << beforeState.devFiles.size() << std::endl;
    std::cout << "  /sys/module files: " << beforeState.sysModuleFiles.size() << std::endl;
    
    // Perform memory operations
    auto pidResult = ukc.findProcessByName("target_app");
    if (pidResult.isSuccess()) {
        pid_t targetPid = pidResult.value();
        auto mapsResult = ukc.getProcessMemoryMaps(targetPid);
        if (mapsResult.isSuccess()) {
            uintptr_t address = mapsResult.value()[0].start;
            ukc.readMemory(targetPid, address, 256);
        }
    }
    
    // Capture system state after operations
    auto afterResult = verifier.captureSystemState();
    if (afterResult.isError()) {
        std::cerr << "Failed to capture system state" << std::endl;
        return 1;
    }
    
    const auto& afterState = afterResult.value();
    std::cout << "Captured system state after operations" << std::endl;
    std::cout << "  Loaded modules: " << afterState.loadedModules.size() << std::endl;
    std::cout << "  /dev files: " << afterState.devFiles.size() << std::endl;
    std::cout << "  /sys/module files: " << afterState.sysModuleFiles.size() << std::endl;
    
    // Verify stealth
    auto stealthResult = verifier.verifyStealthOperation(beforeState, afterState);
    if (stealthResult.isSuccess()) {
        if (stealthResult.value()) {
            std::cout << "✓ Operations were stealthy (no traces left)" << std::endl;
        } else {
            std::cout << "✗ Operations left traces" << std::endl;
        }
    }
    
    return 0;
}
```

## Compilation

To compile these examples, use:

```bash
g++ -std=c++17 -I./include -L./build -o example example.cpp \
    -luserspace_kernel_call -lrapidcheck -lgtest -lpthread
```

Or use CMake:

```bash
mkdir build
cd build
cmake ..
make
./example
```

## Common Patterns

### Pattern 1: Safe Memory Access

```cpp
auto result = ukc.readMemory(pid, address, size);
if (result.isSuccess()) {
    const auto& data = result.value();
    // Process data
} else {
    // Handle error
    std::cerr << "Error: " << result.errorMessage() << std::endl;
}
```

### Pattern 2: Process Discovery

```cpp
auto pidResult = ukc.findProcessByName("app_name");
if (pidResult.isSuccess()) {
    pid_t pid = pidResult.value();
    // Use pid
} else {
    std::cerr << "Process not found" << std::endl;
}
```

### Pattern 3: Memory Mapping

```cpp
auto mapsResult = ukc.getProcessMemoryMaps(pid);
if (mapsResult.isSuccess()) {
    for (const auto& region : mapsResult.value()) {
        std::cout << "Region: 0x" << std::hex << region.start 
                  << " - 0x" << region.end << std::endl;
    }
}
```

## Troubleshooting

### Issue: "Not running as root"

**Solution**: Run the application with root privileges:
```bash
sudo ./application
```

### Issue: "Kernel memory mapping unavailable"

**Solution**: This may indicate:
- Kernel version incompatibility
- SELinux restrictions
- Missing kernel symbols

### Issue: "Process not found"

**Solution**: Verify the process name matches exactly:
```bash
ps aux | grep process_name
```

### Issue: "Invalid address"

**Solution**: Ensure the address is within the process's memory maps:
```cpp
auto mapsResult = ukc.getProcessMemoryMaps(pid);
// Check that your address is within one of the returned regions
```

## Performance Tips

1. **Use Batch Operations**: Combine multiple operations to reduce overhead
2. **Cache Addresses**: Reuse located kernel function addresses
3. **Monitor Performance**: Use PerformanceMonitor to identify bottlenecks
4. **Minimize Data Size**: Read/write only necessary data

## Security Considerations

1. **Verify Processes**: Always verify target process exists and is accessible
2. **Validate Addresses**: Check addresses are within valid ranges
3. **Handle Errors**: Properly handle all error cases
4. **Clean Up Resources**: Ensure all resources are properly released
5. **Verify Stealth**: Use StealthVerifier to confirm operations leave no traces
