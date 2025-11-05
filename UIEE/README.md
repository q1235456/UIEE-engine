# UIEE智能调度引擎 v3.0

基于帕累托最优前沿和纳什均衡算法的智能调度引擎，实现多层优先级调度和体验感知优化。

## ✨ 特性

- 🎯 **智能场景感知**: 自动检测游戏、社交、媒体、办公等使用场景
- 🧠 **帕累托最优算法**: 基于多目标优化找到性能与功耗的最佳平衡点
- ⚖️ **纳什均衡调度**: 实现多任务间的公平资源分配
- 🔧 **CTO任务优化**: 集成CoreTaskOptimizer的任务绑定和CPU亲和性
- 📊 **CES综合评分**: 响应性、流畅性、效率、热管理的综合体验分数
- 🌐 **WebUI管理界面**: 移动端优化的实时配置管理界面
- 📱 **Magisk/Apatch兼容**: 完美支持主流Magisk框架

## 🚀 安装

### 系统要求

- Android 7.0+ (API Level 24+)
- Magisk 20.4+ 或 Apatch 1.0.5+
- ARM64架构设备
- Root权限

### 安装步骤

1. **下载模块**
   ```
   下载 uiee_module_v3.0.zip 文件
   ```

2. **通过Magisk/Apatch安装**
   - 打开Magisk Manager或Apatch管理器
   - 点击"模块" → "从本地安装"
   - 选择下载的zip文件
   - 等待安装完成

3. **重启设备**
   - 安装完成后重启设备以启用模块

4. **验证安装**
   - 检查Magisk/Apatch管理器中的模块列表
   - 确认"UIEE智能调度引擎"显示为已启用状态

## 🎮 使用说明

### WebUI管理界面

模块安装后，可通过以下方式访问WebUI：

```
http://localhost:8080
```

**功能特性：**
- 📊 实时系统状态监控
- 🎯 场景模式切换
- ⚙️ 引擎启停控制
- 🔧 CTO优化设置
- 📝 实时日志查看

### 场景模式

#### 🎮 游戏模式
- 目标FPS: 60
- 优先性能优化
- CPU核心绑定
- 最小功耗限制

#### 💬 社交模式
- 目标FPS: 30
- 平衡性能与功耗
- 智能后台管理
- 温度控制优化

#### 📺 媒体模式
- 目标FPS: 30
- 视频解码优化
- GPU加速启用
- 流畅播放保证

#### 💼 办公模式
- 目标FPS: 60
- 响应性优先
- 多任务优化
- 电池续航优化

### 高级设置

#### CTO任务优化
- **任务绑定**: 将前台应用绑定到高性能核心
- **I/O调度**: 优化磁盘和网络I/O优先级
- **CPU亲和性**: 设置特定核心的CPU亲和性

#### 调度参数
- **调度间隔**: 调整优化检查频率 (1-30秒)
- **优化强度**: 控制优化算法激进程度 (0-100%)

## 🔧 技术架构

### 核心算法

#### 帕累托最优前沿
```cpp
// 多目标优化：性能 vs 功耗 vs 热管理
std::vector<ParetoPoint> calculateParetoFrontier(points);
ParetoPoint findOptimalPoint(frontier);
```

#### 纳什均衡算法
```cpp
// 多任务资源分配的公平性保证
NashEquilibrium calculateNashEquilibrium(payoff_matrix);
```

#### CES综合评分
```cpp
// 综合体验分数计算
double ces_score = 
    responsiveness_weight * responsiveness_score +
    fluency_weight * fluency_score +
    efficiency_weight * efficiency_score -
    thermal_weight * thermal_state;
```

### CTO集成

- **任务识别**: 自动识别前台应用类型
- **CPU绑核**: 将关键任务绑定到高性能核心
- **I/O优化**: 调整磁盘和网络I/O调度策略
- **实时监控**: 持续监控任务状态和性能指标

### 文件结构

```
uiee_module_v3/
├── META-INF/
│   └── com/google/android/
│       ├── update-binary      # 安装脚本
│       └── updater-script     # 更新脚本
├── module.prop                # 模块属性
├── service.sh                 # 服务启动脚本
├── bin/
│   ├── uiee_engine           # C++核心引擎
│   ├── uiee_engine.cpp       # 引擎源码
│   └── main.cpp              # 主程序
├── conf/
│   └── uiee.conf             # 配置文件
├── webroot/
│   ├── index.html            # WebUI主页面
│   ├── styles.css            # 样式文件
│   └── app.js                # 前端逻辑
└── include/
    └── uiee_engine.h         # 引擎头文件
```

## 🛠️ 故障排除

### 常见问题

#### 1. 模块安装失败
**症状**: Magisk/Apatch提示安装失败
**解决**: 
- 检查设备架构是否为ARM64
- 确认Magisk/Apatch版本符合要求
- 重启设备后重试

#### 2. 引擎无法启动
**症状**: WebUI显示引擎未运行
**解决**:
- 检查日志文件: `/data/adb/modules/uiee_smart_engine/logs/`
- 确认二进制文件权限正确
- 手动运行: `/data/adb/modules/uiee_smart_engine/bin/uiee_engine --test`

#### 3. WebUI无法访问
**症状**: 浏览器无法打开WebUI
**解决**:
- 确认端口8080未被占用
- 检查防火墙设置
- 重启WebUI服务

#### 4. 性能无明显改善
**症状**: 使用后性能无明显变化
**解决**:
- 调整场景模式设置
- 检查CTO优化是否启用
- 查看性能日志分析

### 日志文件位置

```
/data/adb/modules/uiee_smart_engine/logs/
├── service.log       # 服务日志
├── engine.log        # 引擎日志
├── error.log         # 错误日志
└── performance.log   # 性能日志
```

### 调试模式

启用调试模式：
```bash
# 编辑配置文件
/data/adb/modules/uiee_smart_engine/data/config/uiee.conf

# 设置日志级别为DEBUG
[logging]
log_level=DEBUG
```

## 📋 更新日志

### v3.0.0 (2024-11-04)
- ✨ 全新C++原生实现，无需Python
- 🎯 集成帕累托最优和纳什均衡算法
- 🔧 集成CTO任务识别和CPU绑核
- 📱 优化移动端WebUI界面
- ⚡ 提升性能和稳定性
- 🔄 改进Magisk/Apatch兼容性

### v2.0.0
- 🧠 AI场景感知算法
- 📊 CES综合评分系统
- 🌐 WebUI管理界面
- 🔧 CTO优化集成

## 🤝 贡献

欢迎提交Issue和Pull Request来改进这个项目。

## 📄 许可证

本项目采用MIT许可证，详见LICENSE文件。

## 🙏 致谢

感谢以下开源项目的启发：
- CoreTaskOptimizer - 任务识别和CPU绑核
- Encore Tweaks - WebUI界面设计
- MAGNETAR - AI/ML场景自适应
- LKT - Linux内核调优

## 📞 联系方式

- 作者: 元昔
- 项目地址: [GitHub Repository]
- 问题反馈: [Issues页面]

---

**注意**: 本模块仅供学习和研究使用，使用前请备份重要数据。开发者不对使用本模块造成的任何损失承担责任。