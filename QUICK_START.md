# ⚡ 快速开始指南

## 🎯 5 分钟快速上手

### 1️⃣ 编译项目

```bash
cd userspace-kernel-call
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

### 2️⃣ 运行测试

```bash
ctest --output-on-failure
```

### 3️⃣ 使用库

```cpp
#include "userspace_kernel_call.h"

using namespace ukc;

int main() {
    UserspaceKernelCall ukc;
    ukc.initialize();
    
    // 查找进程
    auto pid = ukc.findProcessByName("target_app").value();
    
    // 读取内存
    auto data = ukc.readMemory(pid, 0x1000, 256).value();
    
    return 0;
}
```

## 📦 GitHub Actions 自动编译

### 上传到 GitHub

```bash
git init
git add .
git commit -m "Initial commit"
git remote add origin https://github.com/YOUR_USERNAME/userspace-kernel-call.git
git push -u origin main
```

### 查看编译结果

1. 打开 GitHub 仓库
2. 点击 "Actions" 标签
3. 查看编译日志

## 🔧 常用命令

| 命令 | 功能 |
|------|------|
| `cmake ..` | 配置编译 |
| `make -j$(nproc)` | 并行编译 |
| `ctest` | 运行测试 |
| `make clean` | 清理编译 |
| `rm -rf build` | 删除 build 目录 |

## 📚 文档导航

| 文档 | 用途 |
|------|------|
| README.md | 项目概览 |
| BUILD_GUIDE.md | 详细编译 |
| USAGE_EXAMPLES.md | 使用示例 |
| ARCHITECTURE.md | 架构设计 |

## ⚠️ 常见问题

### Q: 编译失败？
**A**: 检查依赖是否安装
```bash
sudo apt-get install build-essential cmake libgtest-dev
```

### Q: 测试失败？
**A**: 查看详细日志
```bash
ctest --output-on-failure -V
```

### Q: 如何只编译库？
**A**: 修改 CMakeLists.txt，注释掉测试部分

## 🚀 下一步

1. 查看 `USAGE_EXAMPLES.md` 了解如何使用
2. 查看 `ARCHITECTURE.md` 了解系统设计
3. 上传到 GitHub 启用自动编译

---

**需要帮助？** 查看完整文档！
