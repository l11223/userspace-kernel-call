# 编译指南 - Userspace Kernel Call

## 🚀 方法 1：使用 GitHub Actions（推荐）

### 步骤：

1. **上传到 GitHub**
   ```bash
   cd userspace-kernel-call
   git init
   git add .
   git commit -m "Initial commit: Userspace Kernel Call"
   git branch -M main
   git remote add origin https://github.com/你的用户名/userspace-kernel-call.git
   git push -u origin main
   ```

2. **自动编译**
   - 推送后，GitHub Actions 会自动开始编译
   - 访问：`https://github.com/你的用户名/userspace-kernel-call/actions`
   - 等待编译完成（约 5-10 分钟）

3. **下载编译好的文件**
   - 点击最新的 workflow run
   - 在 "Artifacts" 部分下载：
     - `userspace-kernel-call-linux-x86_64` - Linux 版本
     - `userspace-kernel-call-android-arm64` - Android ARM64 版本
     - `userspace-kernel-call-android-arm32` - Android ARM32 版本
     - `userspace-kernel-call-release` - 完整发布包

### 编译产物：

```
userspace-kernel-call-release.tar.gz
├── lib/
│   ├── linux-x86_64/
│   │   ├── libuserspace_kernel_call.a    # 静态库
│   │   └── libuserspace_kernel_call.so   # 动态库
│   ├── android-arm64/
│   │   ├── libuserspace_kernel_call.a
│   │   └── libuserspace_kernel_call.so
│   └── android-arm32/
│       ├── libuserspace_kernel_call.a
│       └── libuserspace_kernel_call.so
├── include/                               # 头文件
└── docs/                                  # 文档
```

## 🔧 方法 2：本地编译（Linux/macOS）

### 前置要求：
```bash
# Ubuntu/Debian
sudo apt-get install build-essential cmake

# macOS
brew install cmake
```

### 编译步骤：

```bash
# 1. 创建构建目录
mkdir build
cd build

# 2. 配置
cmake ..

# 3. 编译
make -j$(nproc)

# 4. 查看产物
ls -lh libuserspace_kernel_call.*
```

## 📱 方法 3：Android NDK 交叉编译

### 前置要求：
- 下载 Android NDK: https://developer.android.com/ndk/downloads

### 编译 ARM64：

```bash
export ANDROID_NDK=/path/to/android-ndk-r25c

mkdir build-android
cd build-android

cmake .. \
  -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
  -DANDROID_ABI=arm64-v8a \
  -DANDROID_PLATFORM=android-21 \
  -DCMAKE_BUILD_TYPE=Release

make -j$(nproc)
```

### 编译 ARM32：

```bash
mkdir build-android-arm32
cd build-android-arm32

cmake .. \
  -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
  -DANDROID_ABI=armeabi-v7a \
  -DANDROID_PLATFORM=android-21 \
  -DCMAKE_BUILD_TYPE=Release

make -j$(nproc)
```

## 📦 使用编译好的库

### 静态链接：

```bash
g++ -std=c++17 your_app.cpp \
    -I./include \
    -L./lib/android-arm64 \
    -luserspace_kernel_call \
    -lpthread \
    -o your_app
```

### 动态链接：

```bash
g++ -std=c++17 your_app.cpp \
    -I./include \
    -L./lib/android-arm64 \
    -luserspace_kernel_call \
    -lpthread \
    -o your_app

# 运行时需要设置库路径
export LD_LIBRARY_PATH=./lib/android-arm64:$LD_LIBRARY_PATH
./your_app
```

## 🎯 推送到设备

### 推送到 Android 设备：

```bash
# 推送库文件
adb push lib/android-arm64/libuserspace_kernel_call.so /data/local/tmp/

# 推送你的应用
adb push your_app /data/local/tmp/

# 运行
adb shell "su -c 'cd /data/local/tmp && chmod +x your_app && ./your_app'"
```

## ⚠️ 注意事项

1. **必须 Root 权限**：这个库需要 Root 权限才能工作
2. **架构匹配**：确保使用正确的架构版本（ARM64/ARM32）
3. **SELinux**：某些设备可能需要调整 SELinux 策略
4. **内核版本**：特征码可能需要根据内核版本调整

## 🐛 故障排除

### 编译错误：找不到 pthread
```bash
# 确保链接了 pthread
target_link_libraries(your_app pthread)
```

### 运行时错误：找不到 .so 文件
```bash
# 设置库路径
export LD_LIBRARY_PATH=/path/to/lib:$LD_LIBRARY_PATH
```

### Android 上权限被拒绝
```bash
# 确保有 Root 权限
adb shell su -c "your_command"
```

## 📚 更多信息

- [使用示例](USAGE_EXAMPLES.md)
- [架构文档](ARCHITECTURE.md)
- [API 文档](README.md)
