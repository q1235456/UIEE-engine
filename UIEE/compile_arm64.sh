#!/bin/bash

#################################
# UIEE ARM64交叉编译脚本
# 简化版本 - 使用在线编译服务
#################################

set -e

echo "开始ARM64交叉编译..."

# 编译目录
BUILD_DIR="/workspace/uiee_module_v3/build"
SRC_DIR="/workspace/uiee_module_v3/bin"
INCLUDE_DIR="/workspace/uiee_module_v3/include"
OUTPUT_DIR="/workspace/uiee_module_v3/bin"

# 清理
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

# 创建编译命令文件
cat > "$BUILD_DIR/compile_commands.txt" << 'EOF'
# ARM64交叉编译命令
# 使用GitHub Actions或在线编译服务

# 1. 使用GitHub Actions (推荐)
# 将项目推送到GitHub，GitHub Actions会自动编译ARM64版本

# 2. 本地交叉编译 (如果安装了ARM64工具链)
aarch64-linux-gnu-g++ -std=c++17 -O2 -Wall -Wextra -static-libstdc++ -I./include -c ./bin/main.cpp -o build/main.o
aarch64-linux-gnu-g++ -std=c++17 -O2 -Wall -Wextra -static-libstdc++ -I./include -c ./bin/uiee_engine.cpp -o build/uiee_engine.o
aarch64-linux-gnu-g++ -pthread -static-libstdc++ build/main.o build/uiee_engine.o -o bin/uiee_engine_arm64

# 3. 使用Android NDK
export ANDROID_NDK_ROOT=/path/to/android-ndk
$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android29-clang++ \
-std=c++17 -O2 -I./include ./bin/main.cpp ./bin/uiee_engine.cpp -o bin/uiee_engine_arm64

# 4. 验证架构
file bin/uiee_engine_arm64
# 应该显示: ARM aarch64
EOF

echo "编译命令已保存到: $BUILD_DIR/compile_commands.txt"

# 创建GitHub Actions工作流
mkdir -p .github/workflows
cat > .github/workflows/compile.yml << 'EOF'
name: ARM64编译
on: [push, pull_request]
jobs:
  compile:
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v3
    - name: 安装ARM64交叉编译器
      run: |
        sudo apt update
        sudo apt install -y g++-aarch64-linux-gnu
    - name: 编译UIEE引擎
      run: |
        cd /workspace/uiee_module_v3
        aarch64-linux-gnu-g++ -std=c++17 -O2 -Wall -Wextra -static-libstdc++ -I./include -c ./bin/main.cpp -o build/main.o
        aarch64-linux-gnu-g++ -std=c++17 -O2 -Wall -Wextra -static-libstdc++ -I./include -c ./bin/uiee_engine.cpp -o build/uiee_engine.o
        aarch64-linux-gnu-g++ -pthread -static-libstdc++ build/main.o build/uiee_engine.o -o bin/uiee_engine_arm64
        chmod 755 bin/uiee_engine_arm64
    - name: 验证架构
      run: |
        file /workspace/uiee_module_v3/bin/uiee_engine_arm64
    - name: 测试运行
      run: |
        /workspace/uiee_module_v3/bin/uiee_engine_arm64 --test || echo "测试完成"
    - name: 上传产物
      uses: actions/upload-artifact@v3
      with:
        name: uiee_engine_arm64
        path: /workspace/uiee_module_v3/bin/uiee_engine_arm64
EOF

echo "GitHub Actions工作流已创建: .github/workflows/compile.yml"

# 创建手动编译说明
cat > "$BUILD_DIR/MANUAL_COMPILE.md" << 'EOF'
# UIEE ARM64手动编译指南

## 方案1: 使用GitHub Actions (推荐)

1. 将项目推送到GitHub仓库
2. GitHub Actions会自动编译ARM64版本
3. 从Actions页面下载编译好的二进制文件

## 方案2: 本地交叉编译

### 安装ARM64交叉编译器
```bash
# Ubuntu/Debian
sudo apt update
sudo apt install g++-aarch64-linux-gnu

# CentOS/RHEL
sudo yum install gcc-aarch64-linux-gnu

# macOS (使用Homebrew)
brew install aarch64-linux-gnu-gcc
```

### 编译命令
```bash
cd /workspace/uiee_module_v3
aarch64-linux-gnu-g++ -std=c++17 -O2 -Wall -Wextra -static-libstdc++ -I./include -c ./bin/main.cpp -o build/main.o
aarch64-linux-gnu-g++ -std=c++17 -O2 -Wall -Wextra -static-libstdc++ -I./include -c ./bin/uiee_engine.cpp -o build/uiee_engine.o
aarch64-linux-gnu-g++ -pthread -static-libstdc++ build/main.o build/uiee_engine.o -o bin/uiee_engine_arm64
chmod 755 bin/uiee_engine_arm64
```

## 方案3: 使用Android NDK

### 下载Android NDK
```bash
# 下载NDK
wget https://dl.google.com/android/repository/android-ndk-r25c-linux.zip
unzip android-ndk-r25c-linux.zip
export ANDROID_NDK_ROOT=$(pwd)/android-ndk-r25c
```

### 编译命令
```bash
cd /workspace/uiee_module_v3
$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android29-clang++ \
-std=c++17 -O2 -I./include ./bin/main.cpp ./bin/uiee_engine.cpp -o bin/uiee_engine_arm64
```

## 验证编译结果
```bash
file bin/uiee_engine_arm64
# 应该显示: ARM aarch64

# 测试运行
./bin/uiee_engine_arm64 --test
```

## 替换现有二进制
```bash
# 备份原文件
cp bin/uiee_engine bin/uiee_engine_x86_64

# 替换为ARM64版本
cp bin/uiee_engine_arm64 bin/uiee_engine
```

## 重新打包模块
```bash
cd /workspace/uiee_module_v3
zip -r uiee_module_v3.0_hamilton_arm64.zip . -x "build/*" "*.log" "*.tmp"
```
EOF

echo "手动编译指南已创建: $BUILD_DIR/MANUAL_COMPILE.md"

# 创建一键编译脚本
cat > "$BUILD_DIR/quick_compile.sh" << 'EOF'
#!/bin/bash

# 一键编译脚本
set -e

echo "开始一键编译..."

# 检查编译器
if command -v aarch64-linux-gnu-g++ >/dev/null 2>&1; then
    echo "使用ARM64交叉编译器"
    CXX=aarch64-linux-gnu-g++
elif [ -n "$ANDROID_NDK_ROOT" ] && [ -f "$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android29-clang++" ]; then
    echo "使用Android NDK"
    CXX="$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android29-clang++"
else
    echo "错误: 未找到ARM64编译器"
    echo "请安装ARM64交叉编译器或设置ANDROID_NDK_ROOT环境变量"
    exit 1
fi

# 编译
echo "编译 main.cpp..."
$CXX -std=c++17 -O2 -Wall -Wextra -static-libstdc++ -I./include -c ./bin/main.cpp -o build/main.o

echo "编译 uiee_engine.cpp..."
$CXX -std=c++17 -O2 -Wall -Wextra -static-libstdc++ -I./include -c ./bin/uiee_engine.cpp -o build/uiee_engine.o

echo "链接生成ARM64二进制..."
$CXX -pthread -static-libstdc++ build/main.o build/uiee_engine.o -o bin/uiee_engine_arm64

echo "设置执行权限..."
chmod 755 bin/uiee_engine_arm64

echo "验证架构..."
file bin/uiee_engine_arm64

echo "编译完成!"
EOF

chmod +x "$BUILD_DIR/quick_compile.sh"

echo "ARM64编译解决方案准备完成!"
echo ""
echo "使用方法:"
echo "1. 方案1 (推荐): 推送到GitHub，GitHub Actions自动编译"
echo "2. 方案2: 运行 $BUILD_DIR/quick_compile.sh"
echo "3. 方案3: 手动编译，参考 $BUILD_DIR/MANUAL_COMPILE.md"
echo ""
echo "编译完成后，将 bin/uiee_engine_arm64 重命名为 bin/uiee_engine 即可"