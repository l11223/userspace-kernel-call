cmake_minimum_required(VERSION 3.10)
project(UserspaceKernelCall)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 编译选项
if(NOT MSVC)
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 包含目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 源文件
set(SOURCES
    src/result.cpp
    src/signature_pattern.cpp
    src/signature_scanner.cpp
    src/kernel_function_locator.cpp
    src/arm64_assembly_bridge.cpp
    src/kernel_caller.cpp
    src/process_manager.cpp
    src/memory_injector.cpp
    src/stealth_verifier.cpp
    src/performance_monitor.cpp
    src/userspace_kernel_call.cpp
    src/skroot_interface.cpp
)

# 创建静态库
add_library(userspace_kernel_call STATIC ${SOURCES})

# 创建动态库
add_library(userspace_kernel_call_shared SHARED ${SOURCES})
set_target_properties(userspace_kernel_call_shared PROPERTIES OUTPUT_NAME userspace_kernel_call)

# 链接 pthread（仅在非 Android 平台）
if(NOT ANDROID)
    target_link_libraries(userspace_kernel_call pthread)
    target_link_libraries(userspace_kernel_call_shared pthread)
endif()

# 链接 dl 库（用于动态加载 SKRoot）
target_link_libraries(userspace_kernel_call dl)
target_link_libraries(userspace_kernel_call_shared dl)

# 安装
install(TARGETS userspace_kernel_call userspace_kernel_call_shared
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/ DESTINATION include/userspace_kernel_call)

message(STATUS "Building Userspace Kernel Call library (without tests)")
message(STATUS "To build with tests, install: gtest, rapidcheck, benchmark")
