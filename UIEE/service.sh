#!/system/bin/sh

#################################
# UIEE智能调度引擎服务脚本
# 3.0版本 - C++原生实现
# 无需Python，兼容Magisk/Apatch
#################################

# 确保独立模式
ASH_STANDALONE=1
export ASH_STANDALONE

# 模块路径
MODPATH="${0%/*}"
MODDATA="$MODPATH/data"
MODLOGS="$MODPATH/logs"
MODCONF="$MODPATH/conf"
MODBIN="$MODPATH/bin"

# 日志函数
log_info() {
    echo "[$(date '+%Y-%m-%d %H:%M:%S')] [UIEE-SERVICE] 信息: $1" >> "$MODLOGS/service.log"
}

log_error() {
    echo "[$(date '+%Y-%m-%d %H:%M:%S')] [UIEE-SERVICE] 错误: $1" >> "$MODLOGS/error.log"
    echo "UIEE服务错误: $1"
}

log_warning() {
    echo "[$(date '+%Y-%m-%d %H:%M:%S')] [UIEE-SERVICE] 警告: $1" >> "$MODLOGS/service.log"
    echo "UIEE服务警告: $1"
}

log_success() {
    log_info "成功: $1"
}

# 等待系统启动完成
wait_for_boot() {
    log_info "等待系统启动完成"
    
    local timeout=300  # 5分钟超时
    local elapsed=0
    
    while [ $elapsed -lt $timeout ]; do
        if [ "$(getprop sys.boot_completed)" = "1" ]; then
            log_info "系统启动完成"
            return 0
        fi
        
        sleep 5
        elapsed=$((elapsed + 5))
    done
    
    log_error "等待系统启动超时"
    return 1
}

# 等待数据分区挂载
wait_for_data() {
    log_info "等待数据分区挂载"
    
    local timeout=120  # 2分钟超时
    local elapsed=0
    
    while [ $elapsed -lt $timeout ]; do
        if [ -w "/data" ]; then
            log_info "数据分区已挂载"
            return 0
        fi
        
        sleep 3
        elapsed=$((elapsed + 3))
    done
    
    log_error "等待数据分区挂载超时"
    return 1
}

# 初始化目录
init_directories() {
    log_info "初始化模块目录"
    
    # 确保所有目录存在
    mkdir -p "$MODDATA/config"
    mkdir -p "$MODDATA/cache"
    mkdir -p "$MODDATA/logs"
    mkdir -p "$MODDATA/performance"
    
    # 设置正确权限
    chmod 755 "$MODPATH"
    chmod 755 "$MODDATA"
    chmod 755 "$MODDATA/config"
    chmod 755 "$MODDATA/cache"
    chmod 755 "$MODDATA/logs"
    chmod 755 "$MODDATA/performance"
    
    # 创建日志文件
    touch "$MODLOGS/service.log"
    touch "$MODLOGS/error.log"
    touch "$MODLOGS/performance.log"
    
    log_success "目录初始化完成"
}

# 创建默认配置
create_default_config() {
    log_info "创建默认配置"
    
    cat > "$MODDATA/config/uiee.conf" << 'EOF'
# UIEE智能调度引擎配置
# 3.0版本配置

[system]
# 系统设置
enable_engine=true
scheduling_interval=5
optimization_enabled=true

[ces_calculator]
# 综合体验分数权重
responsiveness_weight=0.3
fluency_weight=0.3
efficiency_weight=0.2
thermal_weight=0.2

[scheduling]
# 调度设置
user_manual_priority=1
preset_mode_priority=2
whitelist_rule_priority=3
auto_smart_priority=4

[scene_perception]
# 场景感知
enable_scene_detection=true
game_fps_target=60
social_fps_target=30
media_fps_target=30
productivity_fps_target=60

[logging]
# 日志设置
log_level=INFO
max_log_size=10
enable_performance_log=true
EOF
    
    log_success "默认配置创建完成"
}

# 加载配置
load_config() {
    log_info "加载配置"
    
    if [ -f "$MODDATA/config/uiee.conf" ]; then
        . "$MODDATA/config/uiee.conf" 2>/dev/null || {
            log_error "加载用户配置失败，使用默认配置"
            create_default_config
        }
    else
        log_info "用户配置不存在，创建默认配置"
        create_default_config
    fi
}

# 检测系统架构并选择合适的二进制文件
detect_architecture() {
    local arch=$(getprop ro.product.cpu.abi 2>/dev/null || getprop ro.cpu.abi 2>/dev/null || echo "unknown")
    log_info "检测到系统架构: $arch"
    
    case "$arch" in
        *arm64*|*aarch64*)
            if [ -f "$MODBIN/uiee_engine_arm64" ] && [ -x "$MODBIN/uiee_engine_arm64" ]; then
                echo "uiee_engine_arm64"
                log_info "选择ARM64二进制文件"
            else
                echo "uiee_engine"
                log_warning "ARM64二进制文件不存在，使用默认文件"
            fi
            ;;
        *arm*|*aarch*)
            echo "uiee_engine"
            log_info "选择ARM二进制文件"
            ;;
        *x86_64*|*amd64*)
            echo "uiee_engine"
            log_info "选择x86-64二进制文件"
            ;;
        *)
            echo "uiee_engine"
            log_info "未知架构，使用默认二进制文件"
            ;;
    esac
}

# 检查二进制文件
check_binaries() {
    log_info "检查二进制文件"
    
    # 检测架构并选择二进制文件
    ENGINE_BINARY=$(detect_architecture)
    log_info "使用二进制文件: $ENGINE_BINARY"
    
    if [ ! -f "$MODBIN/$ENGINE_BINARY" ]; then
        log_error "UIEE核心引擎二进制文件不存在: $MODBIN/$ENGINE_BINARY"
        return 1
    fi
    
    if [ ! -x "$MODBIN/$ENGINE_BINARY" ]; then
        log_error "UIEE核心引擎二进制文件不可执行: $MODBIN/$ENGINE_BINARY"
        return 1
    fi
    
    log_success "二进制文件检查通过: $ENGINE_BINARY"
    return 0
}

# 启动UIEE核心引擎
start_uiee_engine() {
    log_info "启动UIEE核心引擎"
    
    # 检测架构并选择二进制文件
    ENGINE_BINARY=$(detect_architecture)
    
    # 检查二进制文件
    if [ ! -f "$MODBIN/$ENGINE_BINARY" ] || [ ! -x "$MODBIN/$ENGINE_BINARY" ]; then
        log_error "UIEE核心引擎二进制文件不可用: $MODBIN/$ENGINE_BINARY"
        return 1
    fi
    
    # 启动核心引擎
    cd "$MODPATH"
    nohup "$MODBIN/$ENGINE_BINARY" > "$MODLOGS/engine.log" 2>&1 &
    local engine_pid=$!
    
    # 检查进程是否启动成功
    sleep 2
    if ! kill -0 "$engine_pid" 2>/dev/null; then
        log_error "UIEE核心引擎启动失败"
        return 1
    fi
    
    # 保存进程ID
    echo "$engine_pid" > "$MODDATA/engine.pid"
    
    log_success "UIEE核心引擎启动成功，PID: $engine_pid"
    return 0
}

# 启动Web UI服务
start_web_ui() {
    log_info "启动Web UI服务"
    
    # 检查Web UI文件
    if [ ! -f "$MODPATH/webroot/index.html" ]; then
        log_info "Web UI文件不存在，跳过Web UI启动"
        return 0
    fi
    
    # 使用busybox内置的httpd启动Web UI
    cd "$MODPATH/webroot"
    
    # 尝试使用busybox httpd
    if command -v busybox >/dev/null 2>&1; then
        busybox httpd -f -p 8080 -h "$MODPATH/webroot" > "$MODLOGS/web_ui.log" 2>&1 &
        local web_pid=$!
    else
        # 备用方案：使用简单的shell脚本提供Web服务
        start_simple_web_server
        return $?
    fi
    
    # 检查Web UI是否启动成功
    sleep 2
    if kill -0 "$web_pid" 2>/dev/null; then
        echo "$web_pid" > "$MODDATA/web_ui.pid"
        log_success "Web UI服务启动成功，PID: $web_pid，端口: 8080"
    else
        log_error "Web UI服务启动失败"
    fi
    
    cd "$MODPATH"
}

# 简单的Web服务器（备用方案）
start_simple_web_server() {
    log_info "启动简单Web服务器"
    
    # 创建一个简单的shell Web服务器
    while true; do
        # 监听8080端口
        nc -l -p 8080 -e /system/bin/sh -c 'echo -e "HTTP/1.1 200 OK\nContent-Type: text/html\n\n$(cat $MODPATH/webroot/index.html)"' 2>/dev/null || break
        sleep 1
    done &
    
    local web_pid=$!
    echo "$web_pid" > "$MODDATA/web_ui.pid"
    log_success "简单Web服务器启动成功，PID: $web_pid"
}

# 性能监控
performance_monitor() {
    log_info "启动性能监控"
    
    # 简单的性能监控循环
    while true; do
        # 记录CPU使用率
        if [ -f /proc/stat ]; then
            cpu_info=$(head -1 /proc/stat)
            echo "[$(date '+%Y-%m-%d %H:%M:%S')] CPU: $cpu_info" >> "$MODLOGS/performance.log"
        fi
        
        # 记录内存使用情况
        if [ -f /proc/meminfo ]; then
            mem_info=$(head -3 /proc/meminfo)
            echo "[$(date '+%Y-%m-%d %H:%M:%S')] MEMORY: $mem_info" >> "$MODLOGS/performance.log"
        fi
        
        # 等待下次监控
        sleep 30
    done &
    
    local monitor_pid=$!
    echo "$monitor_pid" > "$MODDATA/monitor.pid"
    log_success "性能监控启动成功，PID: $monitor_pid"
}

# 清理函数
cleanup() {
    log_info "清理UIEE服务"
    
    # 停止所有子进程
    for pid_file in "$MODDATA"/*.pid; do
        if [ -f "$pid_file" ]; then
            pid=$(cat "$pid_file")
            if kill -0 "$pid" 2>/dev/null; then
                kill "$pid" 2>/dev/null
                log_info "停止进程: $pid"
            fi
            rm -f "$pid_file"
        fi
    done
    
    log_info "UIEE服务清理完成"
}

# 信号处理
trap cleanup EXIT

#################################
# 主服务启动流程
#################################

log_info "开始启动UIEE智能调度引擎 v3.0"

# 等待系统启动
wait_for_boot || exit 1

# 等待数据分区
wait_for_data || exit 1

# 初始化目录
init_directories

# 加载配置
load_config

# 启动UIEE核心引擎
start_uiee_engine || {
    log_error "核心引擎启动失败，模块将无法正常工作"
    exit 1
}

# 启动Web UI服务
start_web_ui

# 启动性能监控
performance_monitor

log_info "UIEE智能调度引擎启动完成"

# 保持服务运行，监控进程状态
while true; do
    # 检查核心引擎是否还在运行
    if [ -f "$MODDATA/engine.pid" ]; then
        engine_pid=$(cat "$MODDATA/engine.pid")
        if ! kill -0 "$engine_pid" 2>/dev/null; then
            log_info "检测到UIEE核心引擎异常退出，尝试重启"
            start_uiee_engine
        fi
    fi
    
    # 等待下次检查
    sleep 30
done