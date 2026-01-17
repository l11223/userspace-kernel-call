# Architecture: Userspace Kernel Call

## Overview

The Userspace Kernel Call (UKC) system enables direct invocation of kernel functions from user space without loading kernel driver modules. This architecture document describes the system design, component interactions, and implementation details.

## System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                   Application Layer                         │
│              (UserspaceKernelCall API)                      │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│                  Abstraction Layer                          │
│  ┌──────────────────┐  ┌──────────────────┐                │
│  │  MemoryInjector  │  │ ProcessManager   │                │
│  └──────────────────┘  └──────────────────┘                │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│              Kernel Interface Layer                         │
│  ┌──────────────────┐  ┌──────────────────┐                │
│  │ KernelCaller     │  │KernelFunction    │                │
│  │                  │  │Locator           │                │
│  └──────────────────┘  └──────────────────┘                │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│                Low-Level Layer                              │
│  ┌──────────────────┐  ┌──────────────────┐                │
│  │SignatureScanner  │  │ARM64Assembly     │                │
│  │                  │  │Bridge            │                │
│  └──────────────────┘  └──────────────────┘                │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│                  Linux Kernel                               │
│            (Kernel Functions)                               │
└─────────────────────────────────────────────────────────────┘
```

## Component Responsibilities

### 1. Application Layer (UserspaceKernelCall)

**Purpose**: Provides a unified, high-level API for external applications.

**Key Methods**:
- `initialize()`: Initialize all subsystems
- `readMemory()`: Read target process memory
- `writeMemory()`: Write target process memory
- `batchOperations()`: Perform multiple memory operations
- `findProcessByName()`: Locate a process by name
- `getProcessMemoryMaps()`: Get process memory layout

**Responsibilities**:
- Coordinate initialization of all components
- Provide simplified interfaces for common operations
- Handle error propagation and reporting

### 2. Abstraction Layer

#### MemoryInjector

**Purpose**: High-level memory injection interface.

**Key Methods**:
- `initialize()`: Initialize with dependencies
- `readMemory()`: Read target process memory via kernel functions
- `writeMemory()`: Write target process memory via kernel functions
- `batchOperations()`: Perform multiple operations efficiently

**Responsibilities**:
- Validate target process and addresses
- Coordinate with KernelCaller for actual operations
- Handle memory operation results

#### ProcessManager

**Purpose**: Manage target process information.

**Key Methods**:
- `findProcessByName()`: Search for process by name
- `isProcessAlive()`: Check if process exists
- `getMemoryMaps()`: Parse /proc/pid/maps
- `isValidAddress()`: Verify address validity

**Responsibilities**:
- Process discovery and validation
- Memory map parsing and caching
- Address range verification

### 3. Kernel Interface Layer

#### KernelCaller

**Purpose**: Execute kernel functions from user space.

**Key Methods**:
- `initialize()`: Verify Root privileges
- `callKernelFunction()`: Call kernel function with up to 6 parameters
- `prepareKernelContext()`: Setup kernel call environment
- `cleanupKernelContext()`: Cleanup after kernel call

**Responsibilities**:
- Root privilege verification
- Parameter preparation (ARM64 AAPCS)
- Kernel function invocation via assembly bridge
- Return value capture

#### KernelFunctionLocator

**Purpose**: Locate kernel functions by address.

**Key Methods**:
- `initialize()`: Load kernel memory mappings
- `locateFunction()`: Find function by signature pattern
- `isValidKernelAddress()`: Verify kernel address validity
- `cacheAddress()`: Cache located addresses
- `getCachedAddress()`: Retrieve cached addresses

**Responsibilities**:
- Kernel function address resolution
- Signature pattern matching
- Address caching for performance
- Kernel address range validation

### 4. Low-Level Layer

#### SignatureScanner

**Purpose**: Search for byte patterns in memory.

**Key Methods**:
- `scan()`: Search for pattern in buffer
- `scanWithAlignment()`: Search with alignment requirements

**Responsibilities**:
- Pattern matching with wildcards
- Mask-based matching
- Alignment-aware searching

#### ARM64AssemblyBridge

**Purpose**: Implement low-level kernel function calls.

**Key Functions**:
- `kernel_call_bridge()`: Execute kernel function via inline assembly

**Responsibilities**:
- ARM64 AAPCS calling convention implementation
- Register setup and preservation
- Kernel function invocation via BLR instruction

## Data Flow

### Memory Read Operation

```
Application
    ↓
UserspaceKernelCall::readMemory()
    ↓
MemoryInjector::readMemory()
    ├─ ProcessManager::isProcessAlive()
    ├─ ProcessManager::isValidAddress()
    └─ KernelCaller::callKernelFunction()
        ├─ KernelFunctionLocator::locateFunction()
        └─ ARM64AssemblyBridge::kernel_call_bridge()
            ↓
        Linux Kernel (copy_to_user or similar)
            ↓
        Return data to user space
```

### Memory Write Operation

```
Application
    ↓
UserspaceKernelCall::writeMemory()
    ↓
MemoryInjector::writeMemory()
    ├─ ProcessManager::isProcessAlive()
    ├─ ProcessManager::isValidAddress()
    └─ KernelCaller::callKernelFunction()
        ├─ KernelFunctionLocator::locateFunction()
        └─ ARM64AssemblyBridge::kernel_call_bridge()
            ↓
        Linux Kernel (copy_from_user or similar)
            ↓
        Write data to target process
```

## Key Design Patterns

### 1. Result<T> Pattern

All operations that can fail return `Result<T>`:

```cpp
Result<std::vector<uint8_t>> readMemory(...);
if (result.isSuccess()) {
    auto data = result.value();
} else {
    auto error = result.errorMessage();
}
```

### 2. Dependency Injection

Components receive dependencies through initialization:

```cpp
injector_->initialize(locator_, caller_, processManager_);
```

### 3. Caching Strategy

Frequently accessed data is cached:

```cpp
locator_->cacheAddress("function_name", address);
auto cached = locator_->getCachedAddress("function_name");
```

### 4. Validation Layers

Multiple validation layers ensure safety:

1. Process validation (exists, accessible)
2. Address validation (within valid ranges)
3. Permission validation (Root privileges)
4. Kernel address validation (within kernel space)

## Performance Considerations

### 1. Address Caching

Kernel function addresses are cached after first location to avoid repeated searches.

### 2. Batch Operations

Multiple memory operations can be batched to reduce kernel call overhead:

```cpp
std::vector<MemoryOperation> ops;
ops.push_back(readOp);
ops.push_back(writeOp);
injector_->batchOperations(pid, ops);
```

### 3. Memory Mapping Caching

Process memory maps are cached to avoid repeated /proc parsing.

### 4. Performance Monitoring

The PerformanceMonitor tracks operation timing:

```cpp
monitor.startTimer("operation");
// ... perform operation ...
monitor.stopTimer("operation");
auto stats = monitor.getStats("operation");
```

## Security Considerations

### 1. Root Privilege Verification

All kernel operations require Root privileges, verified at initialization.

### 2. Address Validation

All addresses are validated before use:
- Kernel addresses must be within kernel space
- Process addresses must be within process memory maps

### 3. Process Validation

Target processes are verified to exist and be accessible.

### 4. Resource Cleanup

All resources are properly cleaned up, even on error paths.

## Error Handling Strategy

### 1. Error Categories

- **Initialization Errors**: Root privileges, kernel mapping loading
- **Location Errors**: Signature pattern not found
- **Call Errors**: Invalid addresses, kernel function failures
- **Memory Errors**: Process not found, address invalid
- **System Errors**: Memory allocation, system call failures

### 2. Error Recovery

- Automatic retry for transient errors (up to 3 times)
- Fallback to alternative signature patterns
- Graceful degradation when features unavailable

### 3. Error Reporting

All errors include descriptive messages for debugging:

```cpp
if (result.isError()) {
    std::cerr << "Operation failed: " << result.errorMessage() << std::endl;
}
```

## Testing Strategy

### 1. Unit Tests

Test individual components in isolation:
- SignaturePattern parsing and matching
- ProcessManager process discovery
- KernelCaller parameter preparation

### 2. Property-Based Tests

Verify universal properties across many inputs:
- Address validation always rejects invalid addresses
- Batch operations produce same results as individual operations
- Performance meets requirements

### 3. Integration Tests

Test complete workflows:
- Initialize → Locate → Call → Read/Write
- Error handling and recovery
- Resource cleanup

### 4. Device Tests

Test on actual Android 15 device:
- Kernel compatibility
- SELinux policy compliance
- Performance on real hardware

## Extension Points

### 1. Custom Signature Patterns

Applications can define custom patterns for different kernel versions:

```cpp
SignaturePattern pattern = SignaturePattern::fromHexString(
    "1F 20 03 D5 ?? ?? ?? ?? C0 03 5F D6",
    "FF FF FF FF 00 00 00 00 FF FF FF FF"
);
```

### 2. Custom Kernel Functions

Support for calling arbitrary kernel functions:

```cpp
auto result = caller_->callKernelFunction(address, {arg0, arg1, arg2});
```

### 3. Performance Monitoring

Applications can monitor performance:

```cpp
monitor.startTimer("operation");
// ... perform operation ...
monitor.stopTimer("operation");
auto stats = monitor.getStats("operation");
```

### 4. Stealth Verification

Applications can verify operations leave no traces:

```cpp
auto before = verifier.captureSystemState();
// ... perform operations ...
auto after = verifier.captureSystemState();
bool isStealth = verifier.verifyStealthOperation(before, after);
```

## Limitations and Future Work

### Current Limitations

1. **ARM64 Only**: Currently supports ARM64 architecture only
2. **Root Required**: Requires Root privileges
3. **Kernel Version Specific**: Signature patterns may need adjustment for different kernel versions
4. **SELinux**: May be restricted by SELinux policies

### Future Enhancements

1. **Multi-Architecture Support**: Add support for ARM32, x86, x86_64
2. **Privilege Escalation**: Explore methods to work with lower privileges
3. **Kernel Version Detection**: Automatic kernel version detection and pattern selection
4. **SELinux Policy Generation**: Generate compatible SELinux policies
5. **Performance Optimization**: Further optimize batch operations
6. **Distributed Operations**: Support operations across multiple processes

## References

- ARM64 AAPCS: https://github.com/ARM-software/abi-aa/releases
- Linux Kernel Memory Management: https://www.kernel.org/doc/html/latest/vm/
- Android Security: https://source.android.com/security
