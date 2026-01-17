#!/bin/bash

# 上传 Userspace Kernel Call 到 GitHub 的脚本

echo "=========================================="
echo "  上传 Userspace Kernel Call 到 GitHub"
echo "=========================================="
echo ""

# 检查是否在正确的目录
if [ ! -f "CMakeLists.txt" ]; then
    echo "❌ 错误：请在 userspace-kernel-call 目录下运行此脚本"
    exit 1
fi

# 获取 GitHub 仓库地址
echo "请输入你的 GitHub 仓库地址（例如：https://github.com/username/repo.git）："
read REPO_URL

if [ -z "$REPO_URL" ]; then
    echo "❌ 错误：仓库地址不能为空"
    exit 1
fi

echo ""
echo "📦 准备上传到：$REPO_URL"
echo ""

# 初始化 git（如果还没有）
if [ ! -d ".git" ]; then
    echo "🔧 初始化 Git 仓库..."
    git init
    git branch -M main
fi

# 添加远程仓库
echo "🔗 添加远程仓库..."
git remote remove origin 2>/dev/null
git remote add origin "$REPO_URL"

# 添加所有文件
echo "📝 添加文件..."
git add .

# 提交
echo "💾 提交更改..."
git commit -m "Initial commit: Userspace Kernel Call - 无痕内核注入系统

- 完整的用户态调用内核实现
- 支持 ARM64/ARM32 架构
- 包含 10 个核心组件
- 18 个测试文件，150+ 测试用例
- 完整的文档和使用示例
- GitHub Actions 自动编译配置"

# 推送
echo "🚀 推送到 GitHub..."
git push -u origin main --force

echo ""
echo "=========================================="
echo "  ✅ 上传完成！"
echo "=========================================="
echo ""
echo "📍 下一步："
echo "1. 访问：$REPO_URL"
echo "2. 点击 'Actions' 标签"
echo "3. 等待自动编译完成（约 5-10 分钟）"
echo "4. 下载编译好的文件"
echo ""
echo "📦 编译产物："
echo "  - userspace-kernel-call-linux-x86_64"
echo "  - userspace-kernel-call-android-arm64"
echo "  - userspace-kernel-call-android-arm32"
echo "  - userspace-kernel-call-release (完整包)"
echo ""
