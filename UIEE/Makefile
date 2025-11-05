# UIEE智能调度引擎 Makefile
# 3.0版本 - ARM64编译

# 编译器设置（可通过环境变量覆盖）
CXX ?= g++
STRIP ?= strip

# 编译参数（可通过环境变量覆盖）
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -pthread
LDFLAGS ?= -pthread -static-libstdc++

# 目录设置
SRC_DIR = bin
INCLUDE_DIR = include
BUILD_DIR = build
OUTPUT_DIR = bin

# 源文件
SOURCES = $(SRC_DIR)/main.cpp $(SRC_DIR)/uiee_engine.cpp
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# 目标文件（可覆盖，例如: make TARGET=bin/uiee_engine_arm64）
TARGET ?= $(OUTPUT_DIR)/uiee_engine

# 默认目标
all: $(TARGET)

# 创建构建目录
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# 编译目标文件
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	@echo "编译 $<..."
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# 链接目标文件
$(TARGET): $(OBJECTS)
	@echo "链接生成可执行文件..."
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $@
	@echo "剥离调试信息..."
	$(STRIP) $@ || true
	@chmod 755 $@
	@echo "编译完成: $@"

# 便捷目标：使用 aarch64 交叉编译器生成 ARM64 可执行文件
.PHONY: arm64
arm64:
	@echo "使用 aarch64-linux-gnu-g++ 进行 ARM64 交叉编译"
	$(MAKE) CXX=aarch64-linux-gnu-g++ TARGET=$(OUTPUT_DIR)/uiee_engine_arm64 all

# 清理构建文件
clean:
	@echo "清理构建文件..."
	rm -rf $(BUILD_DIR)
	rm -f $(TARGET)
	@echo "清理完成"

# 测试运行
test: $(TARGET)
	@echo "运行测试..."
	./$(TARGET) --test

# 状态检查
status: $(TARGET)
	@echo "检查引擎状态..."
	./$(TARGET) --status

# 版本信息
version:
	@echo "UIEE智能调度引擎 v3.0.0"
	@echo "编译时间: $(shell date)"
	@echo "编译器: $(shell $(CXX) --version | head -1)"

# 帮助信息
help:
	@echo "UIEE智能调度引擎 Makefile"
	@echo ""
	@echo "可用目标:"
	@echo "  all      - 编译生成可执行文件 (默认)"
	@echo "  clean    - 清理构建文件"
	@echo "  test     - 运行测试模式"
	@echo "  status   - 检查引擎状态"
	@echo "  version  - 显示版本信息"
	@echo "  help     - 显示此帮助信息"
	@echo ""
	@echo "环境变量:"
	@echo "  CXX      - C++编译器 (默认: g++)"
	@echo "  CXXFLAGS - 编译参数"
	@echo "  LDFLAGS  - 链接参数"

# 防止make将文件名作为目标
.PHONY: all clean test status version help

# 依赖关系
$(BUILD_DIR)/main.o: $(SRC_DIR)/main.cpp $(INCLUDE_DIR)/uiee_engine.h
$(BUILD_DIR)/uiee_engine.o: $(SRC_DIR)/uiee_engine.cpp $(INCLUDE_DIR)/uiee_engine.h