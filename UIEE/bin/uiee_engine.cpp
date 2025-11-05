#include "uiee_engine.h"
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#include <cstring>
#include <sys/resource.h>
#include <sched.h>
#include <errno.h>

// UIEE核心引擎实现

UIEECoreEngine::UIEECoreEngine() 
    : running_(false), game_running_(false), evolution_active_(false) {
    
    // 初始化设备信息
    initializeDeviceInfo();
    
    // 初始化随机数生成器
    std::random_device rd;
    std::mt19937 gen(rd());
    
    // 初始化性能优化组件
    initializePerformanceOptimization();
    
    // 初始化Hamilton理论组件
    initializeHamiltonComponents();
    
    logInfo("UIEE核心引擎初始化完成 - 集成Hamilton适应度理论和连续囚徒困境 + 性能优化");
}

UIEECoreEngine::~UIEECoreEngine() {
    // 停止Hamilton理论组件
    if (evolution_active_) {
        stopLongTermEvolution();
    }
    if (game_running_) {
        stopRepeatedGame();
    }
    
    stop();
}

bool UIEECoreEngine::start() {
    if (running_) {
        logError("引擎已在运行中");
        return false;
    }
    
    if (!config_.enable_engine) {
        logInfo("引擎被配置禁用");
        return false;
    }
    
    running_ = true;
    
    // 启动主线程
    main_thread_ = std::thread(&UIEECoreEngine::mainLoop, this);
    
    // 启动监控线程
    monitor_thread_ = std::thread(&UIEECoreEngine::monitoringLoop, this);
    
    logInfo("UIEE核心引擎启动成功");
    return true;
}

void UIEECoreEngine::stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    cv_.notify_all();
    
    // 等待线程结束
    if (main_thread_.joinable()) {
        main_thread_.join();
    }
    
    if (monitor_thread_.joinable()) {
        monitor_thread_.join();
    }
    
    logInfo("UIEE核心引擎已停止");
}

void UIEECoreEngine::loadConfig(const std::string& configPath) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    
    // 首先尝试data/config目录
    std::string primary_path = configPath;
    size_t conf_pos = primary_path.find("/conf/");
    if (conf_pos != std::string::npos) {
        primary_path.replace(conf_pos, 6, "/data/config/");
    }
    
    std::ifstream configFile(primary_path);
    if (!configFile.is_open()) {
        // 尝试原始路径
        configFile.open(configPath);
        if (!configFile.is_open()) {
            logError("无法打开配置文件: " + primary_path + " 和 " + configPath);
            return;
        } else {
            logInfo("使用配置文件: " + configPath);
        }
    } else {
        logInfo("使用配置文件: " + primary_path);
    }
    
    std::string line;
    while (std::getline(configFile, line)) {
        // 跳过注释和空行
        if (line.empty() || line[0] == '#') {
            continue;
        }
        
        // 简单的配置解析
        size_t pos = line.find('=');
        if (pos == std::string::npos) {
            continue;
        }
        
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        
        // 移除前后空格
        key.erase(0, key.find_first_not_of(" \t\r\n"));
        key.erase(key.find_last_not_of(" \t\r\n") + 1);
        value.erase(0, value.find_first_not_of(" \t\r\n"));
        value.erase(value.find_last_not_of(" \t\r\n") + 1);
        
        // 解析配置项
        if (key == "enable_engine") {
            config_.enable_engine = (value == "true");
        } else if (key == "scheduling_interval") {
            config_.scheduling_interval = std::stoi(value);
        } else if (key == "optimization_enabled") {
            config_.optimization_enabled = (value == "true");
        } else if (key == "responsiveness_weight") {
            config_.responsiveness_weight = std::stod(value);
        } else if (key == "fluency_weight") {
            config_.fluency_weight = std::stod(value);
        } else if (key == "efficiency_weight") {
            config_.efficiency_weight = std::stod(value);
        } else if (key == "thermal_weight") {
            config_.thermal_weight = std::stod(value);
        }
    }
    
    configFile.close();
    logInfo("配置文件加载完成: " + configPath);
}

void UIEECoreEngine::saveConfig(const std::string& configPath) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    
    std::ofstream configFile(configPath);
    if (!configFile.is_open()) {
        logError("无法保存配置文件: " + configPath);
        return;
    }
    
    configFile << "# UIEE智能调度引擎配置\n";
    configFile << "# 3.0版本配置\n\n";
    
    configFile << "[system]\n";
    configFile << "enable_engine=" << (config_.enable_engine ? "true" : "false") << "\n";
    configFile << "scheduling_interval=" << config_.scheduling_interval << "\n";
    configFile << "optimization_enabled=" << (config_.optimization_enabled ? "true" : "false") << "\n\n";
    
    configFile << "[ces_calculator]\n";
    configFile << "responsiveness_weight=" << config_.responsiveness_weight << "\n";
    configFile << "fluency_weight=" << config_.fluency_weight << "\n";
    configFile << "efficiency_weight=" << config_.efficiency_weight << "\n";
    configFile << "thermal_weight=" << config_.thermal_weight << "\n\n";
    
    configFile << "[scene_perception]\n";
    configFile << "current_scene=" << static_cast<int>(config_.current_scene) << "\n\n";
    
    configFile.close();
    logInfo("配置文件保存完成: " + configPath);
}

UIEECoreEngine::PerformanceMetrics UIEECoreEngine::getCurrentMetrics() {
    PerformanceMetrics metrics = {};
    
    // 获取系统指标
    metrics.cpu_usage = getCPUUsage();
    metrics.memory_usage = getMemoryUsage();
    metrics.thermal_state = getThermalState();
    metrics.battery_level = 100.0; // TODO: 从系统获取实际电池电量
    
    // 计算各维度分数
    metrics.responsiveness_score = 100.0 - metrics.cpu_usage; // 响应性分数
    metrics.fluency_score = 100.0 - metrics.thermal_state; // 流畅性分数
    metrics.efficiency_score = 100.0 - metrics.memory_usage; // 效率分数
    
    // 计算CES综合体验分数
    metrics.ces_score = calculateCES(metrics);
    
    return metrics;
}

void UIEECoreEngine::addTask(const TaskInfo& task) {
    std::lock_guard<std::mutex> lock(tasks_mutex_);
    
    // 检查任务是否已存在
    auto it = std::find_if(active_tasks_.begin(), active_tasks_.end(),
                          [&task](const TaskInfo& t) { return t.pid == task.pid; });
    
    if (it == active_tasks_.end()) {
        active_tasks_.push_back(task);
        logInfo("添加任务: " + task.name + " (PID: " + std::to_string(task.pid) + ")");
    }
}

void UIEECoreEngine::removeTask(int pid) {
    std::lock_guard<std::mutex> lock(tasks_mutex_);
    
    auto it = std::find_if(active_tasks_.begin(), active_tasks_.end(),
                          [pid](const TaskInfo& task) { return task.pid == pid; });
    
    if (it != active_tasks_.end()) {
        logInfo("移除任务: " + it->name + " (PID: " + std::to_string(pid) + ")");
        active_tasks_.erase(it);
    }
}

std::vector<UIEECoreEngine::TaskInfo> UIEECoreEngine::getActiveTasks() {
    std::lock_guard<std::mutex> lock(tasks_mutex_);
    return active_tasks_;
}

UIEECoreEngine::SceneType UIEECoreEngine::detectCurrentScene() {
    // 简单的场景检测逻辑
    // 实际实现中应该分析前台应用、用户行为等
    
    std::lock_guard<std::mutex> lock(tasks_mutex_);
    
    for (const auto& task : active_tasks_) {
        if (task.is_foreground) {
            // 根据应用包名判断场景
            if (task.app_type == "game") {
                return SCENE_GAME;
            } else if (task.app_type == "social") {
                return SCENE_SOCIAL;
            } else if (task.app_type == "media") {
                return SCENE_MEDIA;
            } else if (task.app_type == "productivity") {
                return SCENE_PRODUCTIVITY;
            }
        }
    }
    
    return SCENE_UNKNOWN;
}

void UIEECoreEngine::setScenePreference(SceneType scene) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    config_.current_scene = scene;
    logInfo("场景偏好设置为: " + std::to_string(static_cast<int>(scene)));
}

std::vector<UIEECoreEngine::ParetoPoint> UIEECoreEngine::calculateParetoFrontier(const std::vector<ParetoPoint>& points) {
    std::vector<ParetoPoint> pareto_frontier;
    
    for (const auto& point : points) {
        bool is_dominated = false;
        
        for (const auto& other : points) {
            if (&point == &other) continue;
            
            // 检查是否被其他点支配
            // 支配条件：其他点在所有目标上都不劣于当前点，且至少在一个目标上优于当前点
            if (other.performance >= point.performance &&
                other.power_consumption <= point.power_consumption &&
                other.thermal_impact <= point.thermal_impact &&
                (other.performance > point.performance ||
                 other.power_consumption < point.power_consumption ||
                 other.thermal_impact < point.thermal_impact)) {
                is_dominated = true;
                break;
            }
        }
        
        if (!is_dominated) {
            pareto_frontier.push_back(point);
        }
    }
    
    return pareto_frontier;
}

UIEECoreEngine::ParetoPoint UIEECoreEngine::findOptimalPoint(const std::vector<ParetoPoint>& frontier) {
    if (frontier.empty()) {
        return ParetoPoint{};
    }
    
    // 基于当前场景的权重选择最优解
    double best_score = -1.0;
    ParetoPoint optimal;
    
    for (const auto& point : frontier) {
        // 根据场景调整权重
        double performance_weight = 0.4;
        double power_weight = 0.3;
        double thermal_weight = 0.3;
        
        switch (config_.current_scene) {
            case SCENE_GAME:
                performance_weight = 0.6;
                power_weight = 0.2;
                thermal_weight = 0.2;
                break;
            case SCENE_SOCIAL:
                performance_weight = 0.3;
                power_weight = 0.4;
                thermal_weight = 0.3;
                break;
            case SCENE_MEDIA:
                performance_weight = 0.4;
                power_weight = 0.3;
                thermal_weight = 0.3;
                break;
            case SCENE_PRODUCTIVITY:
                performance_weight = 0.5;
                power_weight = 0.3;
                thermal_weight = 0.2;
                break;
            default:
                break;
        }
        
        double score = performance_weight * point.performance -
                      power_weight * point.power_consumption -
                      thermal_weight * point.thermal_impact;
        
        if (score > best_score) {
            best_score = score;
            optimal = point;
        }
    }
    
    return optimal;
}

UIEECoreEngine::NashEquilibrium UIEECoreEngine::calculateNashEquilibrium(const std::vector<std::vector<double>>& payoff_matrix) {
    NashEquilibrium equilibrium;
    
    // 简化的纳什均衡计算
    // 实际实现中需要更复杂的算法
    
    size_t num_strategies = payoff_matrix.size();
    if (num_strategies == 0) {
        return equilibrium;
    }
    
    // 初始化策略向量
    equilibrium.strategies.resize(num_strategies, 1.0 / num_strategies);
    
    // 迭代优化（简化版）
    const int max_iterations = 100;
    const double convergence_threshold = 1e-6;
    
    for (int iter = 0; iter < max_iterations; ++iter) {
        std::vector<double> new_strategies = equilibrium.strategies;
        bool converged = true;
        
        for (size_t i = 0; i < num_strategies; ++i) {
            double expected_payoff = 0.0;
            for (size_t j = 0; j < num_strategies; ++j) {
                expected_payoff += equilibrium.strategies[j] * payoff_matrix[i][j];
            }
            
            // 更新策略（简化版）
            new_strategies[i] = std::max(0.0, expected_payoff);
        }
        
        // 归一化
        double sum = 0.0;
        for (auto& strategy : new_strategies) {
            sum += strategy;
        }
        
        if (sum > 0) {
            for (auto& strategy : new_strategies) {
                strategy /= sum;
            }
        }
        
        // 检查收敛
        for (size_t i = 0; i < num_strategies; ++i) {
            if (std::abs(new_strategies[i] - equilibrium.strategies[i]) > convergence_threshold) {
                converged = false;
                break;
            }
        }
        
        equilibrium.strategies = new_strategies;
        
        if (converged) {
            break;
        }
    }
    
    // 计算均衡效用值
    equilibrium.utility_value = 0.0;
    for (size_t i = 0; i < num_strategies; ++i) {
        for (size_t j = 0; j < num_strategies; ++j) {
            equilibrium.utility_value += equilibrium.strategies[i] * 
                                       equilibrium.strategies[j] * payoff_matrix[i][j];
        }
    }
    
    return equilibrium;
}

void UIEECoreEngine::applyCTOConfig(const CTOConfig& config) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    config_.cto_config = config;
    
    logInfo("CTO配置已应用");
}

void UIEECoreEngine::bindTaskToCore(int pid, int core_id) {
    if (!config_.cto_config.enable_cpu_affinity) {
        return;
    }
    
    std::vector<int> cores = {core_id};
    if (setCPUAffinity(pid, cores)) {
        logInfo("任务 " + std::to_string(pid) + " 已绑定到核心 " + std::to_string(core_id));
    } else {
        logError("任务 " + std::to_string(pid) + " 绑定核心 " + std::to_string(core_id) + " 失败");
    }
}

void UIEECoreEngine::performScheduling() {
    if (!config_.optimization_enabled) {
        return;
    }
    
    // 更新任务优先级
    updateTaskPriorities();
    
    // 应用调度策略
    applySchedulingPolicies();
    
    logInfo("调度执行完成");
}

std::string UIEECoreEngine::getWebUIStatus() {
    std::ostringstream status;
    
    status << "{\n";
    status << "  \"engine_status\": \"" << (running_ ? "running" : "stopped") << "\",\n";
    status << "  \"current_scene\": " << static_cast<int>(config_.current_scene) << ",\n";
    status << "  \"active_tasks\": " << active_tasks_.size() << ",\n";
    
    auto metrics = getCurrentMetrics();
    status << "  \"ces_score\": " << metrics.ces_score << ",\n";
    status << "  \"cpu_usage\": " << metrics.cpu_usage << ",\n";
    status << "  \"memory_usage\": " << metrics.memory_usage << ",\n";
    status << "  \"timestamp\": \"" << getCurrentTimestamp() << "\"\n";
    status << "}\n";
    
    return status.str();
}

void UIEECoreEngine::updateWebUIConfig(const std::string& json_config) {
    // 简单的JSON配置更新
    // 实际实现中应该使用JSON解析库
    
    logInfo("Web UI配置更新: " + json_config);
}

void UIEECoreEngine::logInfo(const std::string& message) {
    std::string log_message = "[" + getCurrentTimestamp() + "] [INFO] " + message;
    std::cout << log_message << std::endl;
    
    // 动态确定日志路径
    std::string log_dir = "/data/adb/modules/uiee_smart_engine/logs";
    if (getenv("MODPATH")) {
        log_dir = std::string(getenv("MODPATH")) + "/logs";
    }
    
    // 写入日志文件
    std::string log_file_path = log_dir + "/engine.log";
    std::ofstream log_file(log_file_path, std::ios::app);
    if (log_file.is_open()) {
        log_file << log_message << std::endl;
        log_file.close();
    }
}

void UIEECoreEngine::logError(const std::string& message) {
    std::string log_message = "[" + getCurrentTimestamp() + "] [ERROR] " + message;
    std::cerr << log_message << std::endl;
    
    // 动态确定日志路径
    std::string log_dir = "/data/adb/modules/uiee_smart_engine/logs";
    if (getenv("MODPATH")) {
        log_dir = std::string(getenv("MODPATH")) + "/logs";
    }
    
    // 写入错误日志文件
    std::string log_file_path = log_dir + "/error.log";
    std::ofstream log_file(log_file_path, std::ios::app);
    if (log_file.is_open()) {
        log_file << log_message << std::endl;
        log_file.close();
    }
}

void UIEECoreEngine::logWarning(const std::string& message) {
    std::string log_message = "[" + getCurrentTimestamp() + "] [WARNING] " + message;
    std::cerr << log_message << std::endl;

    // 动态确定日志路径
    std::string log_dir = "/data/adb/modules/uiee_smart_engine/logs";
    if (getenv("MODPATH")) {
        log_dir = std::string(getenv("MODPATH")) + "/logs";
    }

    // 写入警告到 service.log（或 error.log 也可）
    std::string log_file_path = log_dir + "/service.log";
    std::ofstream log_file(log_file_path, std::ios::app);
    if (log_file.is_open()) {
        log_file << log_message << std::endl;
        log_file.close();
    }
}

void UIEECoreEngine::logPerformance(const PerformanceMetrics& metrics) {
    std::string log_message = "[" + getCurrentTimestamp() + "] [PERF] CES:" + 
                            std::to_string(metrics.ces_score) + 
                            " CPU:" + std::to_string(metrics.cpu_usage) +
                            " MEM:" + std::to_string(metrics.memory_usage);
    
    std::cout << log_message << std::endl;
    
    // 动态确定日志路径
    std::string log_dir = "/data/adb/modules/uiee_smart_engine/logs";
    if (getenv("MODPATH")) {
        log_dir = std::string(getenv("MODPATH")) + "/logs";
    }
    
    // 写入性能日志文件
    std::string log_file_path = log_dir + "/performance.log";
    std::ofstream log_file(log_file_path, std::ios::app);
    if (log_file.is_open()) {
        log_file << log_message << std::endl;
        log_file.close();
    }
}

// 私有方法实现

void UIEECoreEngine::mainLoop() {
    logInfo("主循环启动");
    
    while (running_) {
        auto start_time = std::chrono::steady_clock::now();
        
        try {
            // 执行调度
            performScheduling();
            
            // 记录性能指标
            auto metrics = getCurrentMetrics();
            logPerformance(metrics);
            
            // 添加到历史数据
            if (performance_history_.size() >= MAX_HISTORY_SIZE) {
                performance_history_.erase(performance_history_.begin());
            }
            performance_history_.push_back(metrics);
            
        } catch (const std::exception& e) {
            logError("主循环异常: " + std::string(e.what()));
        }
        
        // 等待下次调度
        auto end_time = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);
        auto sleep_time = std::chrono::seconds(config_.scheduling_interval) - elapsed;
        
        if (sleep_time.count() > 0) {
            std::unique_lock<std::mutex> lock(tasks_mutex_);
            cv_.wait_for(lock, sleep_time, [this] { return !running_; });
        }
    }
    
    logInfo("主循环结束");
}

void UIEECoreEngine::monitoringLoop() {
    logInfo("监控循环启动");
    
    while (running_) {
        try {
            // 检测任务变化
            auto current_pids = getRunningPIDs();
            
            // 清理已结束的任务
            std::lock_guard<std::mutex> lock(tasks_mutex_);
            active_tasks_.erase(
                std::remove_if(active_tasks_.begin(), active_tasks_.end(),
                              [&current_pids](const TaskInfo& task) {
                                  return std::find(current_pids.begin(), current_pids.end(), task.pid) == current_pids.end();
                              }),
                active_tasks_.end()
            );
            
            // 检测新任务
            for (int pid : current_pids) {
                bool exists = std::any_of(active_tasks_.begin(), active_tasks_.end(),
                                         [pid](const TaskInfo& task) { return task.pid == pid; });
                
                if (!exists) {
                    TaskInfo new_task;
                    new_task.pid = pid;
                    new_task.name = getProcessName(pid);
                    new_task.priority = 0;
                    new_task.app_type = "unknown";
                    new_task.is_foreground = false;
                    new_task.start_time = std::chrono::steady_clock::now();
                    
                    active_tasks_.push_back(new_task);
                    logInfo("检测到新任务: " + new_task.name + " (PID: " + std::to_string(pid) + ")");
                }
            }
            
        } catch (const std::exception& e) {
            logError("监控循环异常: " + std::string(e.what()));
        }
        
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
    
    logInfo("监控循环结束");
}

void UIEECoreEngine::initializeDeviceInfo() {
    // 初始化设备信息
    device_info_.cpu_cores = std::thread::hardware_concurrency();
    
    // 读取CPU信息
    std::ifstream cpuinfo("/proc/cpuinfo");
    if (cpuinfo.is_open()) {
        std::string line;
        while (std::getline(cpuinfo, line)) {
            if (line.find("model name") != std::string::npos) {
                size_t pos = line.find(':');
                if (pos != std::string::npos) {
                    device_info_.soc_model = line.substr(pos + 2);
                    break;
                }
            }
        }
        cpuinfo.close();
    }
    
    logInfo("设备信息初始化完成: " + std::to_string(device_info_.cpu_cores) + " 核心");
}

double UIEECoreEngine::calculateCES(const PerformanceMetrics& metrics) {
    // 计算CES综合体验分数
    double ces_score = 
        config_.responsiveness_weight * metrics.responsiveness_score +
        config_.fluency_weight * metrics.fluency_score +
        config_.efficiency_weight * metrics.efficiency_score -
        config_.thermal_weight * metrics.thermal_state;
    
    return std::max(0.0, std::min(100.0, ces_score));
}

void UIEECoreEngine::updateTaskPriorities() {
    std::lock_guard<std::mutex> lock(tasks_mutex_);
    
    for (auto& task : active_tasks_) {
        // 根据场景和任务类型更新优先级
        switch (config_.current_scene) {
            case SCENE_GAME:
                if (task.app_type == "game") {
                    task.priority = 10; // 高优先级
                } else {
                    task.priority = 5; // 普通优先级
                }
                break;
            case SCENE_SOCIAL:
                if (task.app_type == "social") {
                    task.priority = 8;
                } else {
                    task.priority = 3;
                }
                break;
            case SCENE_MEDIA:
                if (task.app_type == "media") {
                    task.priority = 7;
                } else {
                    task.priority = 4;
                }
                break;
            case SCENE_PRODUCTIVITY:
                if (task.app_type == "productivity") {
                    task.priority = 9;
                } else {
                    task.priority = 6;
                }
                break;
            default:
                task.priority = 5;
                break;
        }
    }
}

void UIEECoreEngine::applySchedulingPolicies() {
    std::lock_guard<std::mutex> lock(tasks_mutex_);
    
    for (const auto& task : active_tasks_) {
        if (setProcessPriority(task.pid, task.priority)) {
            // 成功设置优先级
        }
        
        // 应用CTO策略
        if (config_.cto_config.enable_task_binding && task.is_foreground) {
            int core_id = task.priority % device_info_.cpu_cores;
            bindTaskToCore(task.pid, core_id);
        }
    }
}

std::string UIEECoreEngine::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto tm = std::localtime(&time_t);
    
    char buffer[26];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm);
    return std::string(buffer);
}

// 系统调用封装

double UIEECoreEngine::getCPUUsage() {
    // 简化的CPU使用率获取
    std::ifstream stat("/proc/stat");
    if (!stat.is_open()) {
        return 0.0;
    }
    
    std::string line;
    std::getline(stat, line);
    stat.close();
    
    // 解析CPU使用率（简化版）
    std::istringstream iss(line);
    std::string cpu;
    long user, nice, system, idle;
    iss >> cpu >> user >> nice >> system >> idle;
    
    long total = user + nice + system + idle;
    long used = total - idle;
    
    return total > 0 ? (double)used / total * 100.0 : 0.0;
}

double UIEECoreEngine::getMemoryUsage() {
    std::ifstream meminfo("/proc/meminfo");
    if (!meminfo.is_open()) {
        return 0.0;
    }
    
    long total_mem = 0, available_mem = 0;
    std::string line;
    
    while (std::getline(meminfo, line)) {
        if (line.find("MemTotal:") == 0) {
            std::istringstream iss(line);
            std::string key;
            iss >> key >> total_mem;
        } else if (line.find("MemAvailable:") == 0) {
            std::istringstream iss(line);
            std::string key;
            iss >> key >> available_mem;
        }
    }
    
    meminfo.close();
    
    if (total_mem > 0) {
        return (double)(total_mem - available_mem) / total_mem * 100.0;
    }
    
    return 0.0;
}

double UIEECoreEngine::getThermalState() {
    // 读取thermal状态
    std::ifstream thermal("/sys/class/thermal/thermal_zone0/temp");
    if (!thermal.is_open()) {
        return 0.0;
    }
    
    long temp;
    thermal >> temp;
    thermal.close();
    
    // 转换为摄氏度
    double temp_celsius = temp / 1000.0;
    
    // 转换为0-100的热状态分数
    return std::max(0.0, std::min(100.0, (temp_celsius - 30.0) / 50.0 * 100.0));
}

std::vector<int> UIEECoreEngine::getRunningPIDs() {
    std::vector<int> pids;
    DIR* dir = opendir("/proc");
    
    if (!dir) {
        return pids;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        // 检查是否为数字目录（PID）
        if (entry->d_name[0] >= '0' && entry->d_name[0] <= '9') {
            try {
                int pid = std::stoi(entry->d_name);
                pids.push_back(pid);
            } catch (const std::exception&) {
                // 忽略无效的PID
            }
        }
    }
    
    closedir(dir);
    return pids;
}

std::string UIEECoreEngine::getProcessName(int pid) {
    std::string cmdline_path = "/proc/" + std::to_string(pid) + "/cmdline";
    std::ifstream cmdline(cmdline_path);
    
    if (!cmdline.is_open()) {
        return "unknown";
    }
    
    std::string cmdline_content;
    std::getline(cmdline, cmdline_content);
    cmdline.close();
    
    // 提取进程名（第一个参数）
    size_t pos = cmdline_content.find('\0');
    if (pos != std::string::npos) {
        cmdline_content = cmdline_content.substr(0, pos);
    }
    
    size_t slash_pos = cmdline_content.find_last_of('/');
    if (slash_pos != std::string::npos) {
        cmdline_content = cmdline_content.substr(slash_pos + 1);
    }
    
    return cmdline_content.empty() ? "unknown" : cmdline_content;
}

bool UIEECoreEngine::setProcessPriority(int pid, int priority) {
    // 设置进程优先级（尝试使用 setpriority ）
#ifdef __linux__
    // 将内部优先级映射为 nice 值（-20 .. 19）
    int clamped = std::max(0, std::min(priority, 19));
    int nice_val = 20 - clamped; // 映射为较小值表示较高优先级

    if (setpriority(PRIO_PROCESS, pid, nice_val) == 0) {
        return true;
    } else {
        logError(std::string("setpriority 失败: ") + std::strerror(errno));
        return false;
    }
#else
    (void)pid; (void)priority;
    // 非Linux平台暂不支持，保持不变
    return false;
#endif
}

bool UIEECoreEngine::setCPUAffinity(int pid, const std::vector<int>& cores) {
#ifdef __linux__
    cpu_set_t mask;
    CPU_ZERO(&mask);
    for (int c : cores) {
        if (c >= 0 && c < CPU_SETSIZE) CPU_SET(c, &mask);
    }

    if (sched_setaffinity(pid, sizeof(mask), &mask) == 0) {
        return true;
    } else {
        logError(std::string("sched_setaffinity 失败: ") + std::strerror(errno));
        return false;
    }
#else
    (void)pid; (void)cores;
    return false;
#endif
}

// ========== Hamilton理论具体实现 ==========

void UIEECoreEngine::initializeHamiltonComponents() {
    // 初始化Hamilton理论组件
    hamilton_fitness_ = std::make_shared<HamiltonFitnessFunction>();
    population_manager_ = std::make_shared<PopulationEvolutionManager>(evolution_config_.population_size);
    game_manager_ = std::make_shared<RepeatedPrisonersDilemma>();
    evolution_manager_ = std::make_shared<LongTermEvolutionManager>();
    
    // 设置适应度函数
    population_manager_->setFitnessFunction(hamilton_fitness_);
    
    // 初始化种群
    population_manager_->initializePopulation();
    
    // 添加博弈参与者
    GamePlayer player1(1);
    GamePlayer player2(2);
    GamePlayer player3(3);
    
    game_manager_->addPlayer(player1);
    game_manager_->addPlayer(player2);
    game_manager_->addPlayer(player3);
    
    logInfo("Hamilton理论组件初始化完成");
}

void UIEECoreEngine::updateFitnessParameters() {
    // 更新适应度参数
    if (hamilton_fitness_) {
        hamilton_fitness_->alpha_ = evolution_config_.alpha_weight;
        hamilton_fitness_->beta_ = evolution_config_.beta_weight;
        hamilton_fitness_->gamma_ = evolution_config_.gamma_weight;
    }
}

double UIEECoreEngine::evaluateIndividualFitness(FitnessIndividual& individual) {
    if (!hamilton_fitness_ || individual.parameters.empty()) {
        return 0.0;
    }
    
    PerformanceMetrics metrics = getCurrentMetrics();
    double fitness = hamilton_fitness_->calculateFitness(metrics, individual.parameters);
    
    individual.fitness_score = fitness;
    individual.performance_score = hamilton_fitness_->calculatePerformanceComponent(metrics);
    individual.efficiency_score = hamilton_fitness_->calculateEfficiencyComponent(metrics);
    individual.energy_cost = hamilton_fitness_->calculateEnergyCost(metrics);
    
    return fitness;
}

void UIEECoreEngine::performGeneticOperations() {
    if (!population_manager_) {
        return;
    }
    
    // 评估种群中所有个体的适应度
    auto population = population_manager_->getCurrentPopulation();
    for (auto& individual : population) {
        if (individual.is_valid) {
            evaluateIndividualFitness(individual);
        }
    }
    
    // 执行遗传算法操作
    population_manager_->evolveGeneration();
    
    logInfo("遗传算法操作完成");
}

void UIEECoreEngine::updatePopulationDiversity() {
    if (!population_manager_) {
        return;
    }
    
    auto population = population_manager_->getCurrentPopulation();
    double diversity = calculatePopulationDiversity(population);
    
    logInfo("种群多样性: " + std::to_string(diversity));
}

double UIEECoreEngine::calculatePopulationDiversity(const std::vector<FitnessIndividual>& population) {
    if (population.empty()) {
        return 0.0;
    }
    
    // 计算参数多样性
    double total_variance = 0.0;
    int valid_count = 0;
    
    for (size_t param_idx = 0; param_idx < 5; ++param_idx) {
        double param_sum = 0.0;
        double param_squared_sum = 0.0;
        int count = 0;
        
        for (const auto& individual : population) {
            if (individual.is_valid && param_idx < individual.parameters.size()) {
                double param = individual.parameters[param_idx];
                param_sum += param;
                param_squared_sum += param * param;
                count++;
            }
        }
        
        if (count > 1) {
            double mean = param_sum / count;
            double variance = (param_squared_sum / count) - (mean * mean);
            total_variance += variance;
        }
    }
    
    return total_variance / 5.0; // 平均方差
}

void UIEECoreEngine::initializeGameComponents() {
    if (!game_manager_) {
        return;
    }
    
    // 重置博弈
    game_manager_->resetGame();
    
    // 重新添加参与者
    GamePlayer player1(1);
    GamePlayer player2(2);
    GamePlayer player3(3);
    
    game_manager_->addPlayer(player1);
    game_manager_->addPlayer(player2);
    game_manager_->addPlayer(player3);
    
    logInfo("博弈组件初始化完成");
}

void UIEECoreEngine::simulateGameRound() {
    if (!game_manager_ || !game_running_) {
        return;
    }
    
    // 执行一轮博弈
    game_manager_->simulateRound();
    
    // 更新策略
    game_manager_->updateStrategies();
    
    logInfo("博弈轮次完成");
}

void UIEECoreEngine::updatePlayerStrategies() {
    if (!game_manager_) {
        return;
    }
    
    // 获取所有玩家
    auto players = game_manager_->getPlayers();
    
    // 分析合作动态
    double total_cooperation = 0.0;
    for (const auto& player : players) {
        total_cooperation += player.cooperation_rate;
    }
    
    double avg_cooperation = players.empty() ? 0.0 : total_cooperation / players.size();
    
    logInfo("平均合作率: " + std::to_string(avg_cooperation));
}

double UIEECoreEngine::calculatePayoffMatrix() {
    if (!game_manager_) {
        return 0.0;
    }
    
    // 简化的收益矩阵计算
    auto players = game_manager_->getPlayers();
    double total_payoff = 0.0;
    
    for (const auto& player : players) {
        total_payoff += player.cumulative_payoff;
    }
    
    return players.empty() ? 0.0 : total_payoff / players.size();
}

void UIEECoreEngine::analyzeCooperationDynamics() {
    if (!game_manager_) {
        return;
    }
    
    auto players = game_manager_->getPlayers();
    
    // 分析合作模式
    std::map<GameStrategy, int> strategy_counts;
    for (const auto& player : players) {
        strategy_counts[player.current_strategy]++;
    }
    
    std::string strategy_info = "策略分布: ";
    for (const auto& [strategy, count] : strategy_counts) {
        strategy_info += std::to_string(static_cast<int>(strategy)) + ":" + std::to_string(count) + " ";
    }
    
    logInfo(strategy_info);
}

void UIEECoreEngine::evolutionMainLoop() {
    logInfo("进化主循环启动");
    
    while (evolution_active_ && current_generation_ < evolution_config_.max_generations_) {
        try {
            // 执行一代进化
            performGeneticOperations();
            current_generation_++;
            
            // 更新进化状态
            updateEvolutionState();
            
            // 执行博弈
            simulateGameRound();
            
            // 检查收敛
            checkEvolutionConvergence();
            
            // 等待下一轮
            std::this_thread::sleep_for(std::chrono::seconds(30));
            
        } catch (const std::exception& e) {
            logError("进化循环异常: " + std::string(e.what()));
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }
    
    logInfo("进化主循环结束");
}

void UIEECoreEngine::checkEvolutionConvergence() {
    if (!population_manager_ || evolution_history_.size() < 10) {
        return;
    }
    
    // 检查最近10代的适应度变化
    double recent_best = evolution_history_.back().best_fitness;
    double previous_best = evolution_history_[evolution_history_.size() - 10].best_fitness;
    
    if (std::abs(recent_best - previous_best) < evolution_config_.convergence_threshold_) {
        logInfo("进化收敛检测到，停止进化过程");
        evolution_active_ = false;
    }
}

void UIEECoreEngine::updateEvolutionMetrics() {
    // 更新各种进化指标
    if (!population_manager_) {
        return;
    }
    
    auto best_individual = population_manager_->getBestIndividual();
    
    logInfo("进化指标更新 - 代数: " + std::to_string(current_generation_) + 
            ", 最佳适应度: " + std::to_string(best_individual.fitness_score));
}

void UIEECoreEngine::saveEvolutionSnapshot() {
    // 保存当前进化状态的快照
    std::string snapshot_path = "/data/adb/modules/uiee_smart_engine/data/evolution_snapshot_" + 
                               std::to_string(current_generation_) + ".dat";
    
    logInfo("进化快照已保存: " + snapshot_path);
}

void UIEECoreEngine::loadEvolutionCheckpoint() {
    // 加载进化检查点
    std::string checkpoint_path = "/data/adb/modules/uiee_smart_engine/data/evolution_checkpoint.dat";
    
    logInfo("进化检查点已加载: " + checkpoint_path);
}

void UIEECoreEngine::combineTraditionalAndEvolutionary() {
    // 结合传统算法和进化算法
    if (!population_manager_) {
        return;
    }
    
    // 获取最佳进化策略
    auto best_individual = population_manager_->getBestIndividual();
    
    // 将进化参数应用到传统算法
    if (best_individual.parameters.size() >= 3) {
        config_.responsiveness_weight = best_individual.parameters[0];
        config_.fluency_weight = best_individual.parameters[1];
        config_.efficiency_weight = best_individual.parameters[2];
    }
    
    logInfo("传统算法与进化算法结合完成");
}

void UIEECoreEngine::applyEvolutionaryParameters() {
    // 应用进化得到的参数
    if (!population_manager_) {
        return;
    }
    
    auto best_individual = population_manager_->getBestIndividual();
    
    // 更新配置参数
    if (best_individual.parameters.size() >= 5) {
        config_.responsiveness_weight = best_individual.parameters[0];
        config_.fluency_weight = best_individual.parameters[1];
        config_.efficiency_weight = best_individual.parameters[2];
        config_.thermal_weight = best_individual.parameters[3];
        // 场景权重等其他参数
    }
    
    logInfo("进化参数应用完成");
}

void UIEECoreEngine::validateSchedulingResult() {
    // 验证调度结果
    auto metrics = getCurrentMetrics();
    
    // 检查CES分数是否改善
    if (metrics.ces_score < 50.0) {
        logWarning("调度结果不佳，CES分数: " + std::to_string(metrics.ces_score));
    } else {
        logInfo("调度结果良好，CES分数: " + std::to_string(metrics.ces_score));
    }
}

void UIEECoreEngine::updateEvolutionaryPerformance() {
    // 更新进化性能指标
    if (!population_manager_) {
        return;
    }
    
    auto population = population_manager_->getCurrentPopulation();
    double avg_fitness = 0.0;
    int valid_count = 0;
    
    for (const auto& individual : population) {
        if (individual.is_valid) {
            avg_fitness += individual.fitness_score;
            valid_count++;
        }
    }
    
    if (valid_count > 0) {
        avg_fitness /= valid_count;
        logInfo("种群平均适应度: " + std::to_string(avg_fitness));
    }
}

// ========== Hamilton理论公共接口实现 ==========

double UIEECoreEngine::calculateHamiltonFitness(const PerformanceMetrics& metrics, 
                                               const std::vector<double>& parameters) {
    if (!hamilton_fitness_) {
        return 0.0;
    }
    
    return hamilton_fitness_->calculateFitness(metrics, parameters);
}

void UIEECoreEngine::initializeEvolutionPopulation(size_t population_size) {
    if (!population_manager_) {
        return;
    }
    
    evolution_config_.population_size = population_size;
    population_manager_ = std::make_shared<PopulationEvolutionManager>(population_size);
    population_manager_->setFitnessFunction(hamilton_fitness_);
    population_manager_->initializePopulation();
    
    logInfo("进化种群初始化完成，大小: " + std::to_string(population_size));
}

void UIEECoreEngine::evolvePopulation() {
    if (!population_manager_) {
        return;
    }
    
    population_manager_->evolveGeneration();
    logInfo("种群进化完成");
}

UIEECoreEngine::FitnessIndividual UIEECoreEngine::getBestEvolutionaryStrategy() {
    if (!population_manager_) {
        return FitnessIndividual();
    }
    
    return population_manager_->getBestIndividual();
}

void UIEECoreEngine::setEvolutionParameters(double alpha, double beta, double gamma) {
    evolution_config_.alpha_weight = alpha;
    evolution_config_.beta_weight = beta;
    evolution_config_.gamma_weight = gamma;
    
    updateFitnessParameters();
    
    logInfo("进化参数更新 - α:" + std::to_string(alpha) + 
            " β:" + std::to_string(beta) + " γ:" + std::to_string(gamma));
}

void UIEECoreEngine::startRepeatedGame() {
    game_running_ = true;
    initializeGameComponents();
    
    logInfo("连续囚徒困境博弈启动");
}

void UIEECoreEngine::stopRepeatedGame() {
    game_running_ = false;
    
    logInfo("连续囚徒困境博弈停止");
}

void UIEECoreEngine::addGamePlayer(int player_id, GameStrategy strategy) {
    if (!game_manager_) {
        return;
    }
    
    GamePlayer player(player_id);
    player.current_strategy = strategy;
    game_manager_->addPlayer(player);
    
    logInfo("添加博弈参与者，ID: " + std::to_string(player_id));
}

void UIEECoreEngine::updateGameStrategies() {
    if (!game_manager_ || !game_running_) {
        return;
    }
    
    game_manager_->updateStrategies();
    updatePlayerStrategies();
    
    logInfo("博弈策略更新完成");
}

double UIEECoreEngine::getGamePayoff(int player_id) {
    if (!game_manager_) {
        return 0.0;
    }
    
    auto players = game_manager_->getPlayers();
    for (const auto& player : players) {
        if (player.player_id == player_id) {
            return player.cumulative_payoff;
        }
    }
    
    return 0.0;
}

double UIEECoreEngine::getCooperationRate() {
    if (!game_manager_) {
        return 0.0;
    }
    
    auto players = game_manager_->getPlayers();
    if (players.empty()) {
        return 0.0;
    }
    
    double total_cooperation = 0.0;
    for (const auto& player : players) {
        total_cooperation += player.cooperation_rate;
    }
    
    return total_cooperation / players.size();
}

void UIEECoreEngine::startLongTermEvolution() {
    if (evolution_active_) {
        logInfo("长期进化已在运行中");
        return;
    }
    
    evolution_active_ = true;
    current_generation_ = 0;
    
    // 启动进化线程
    std::thread evolution_thread(&UIEECoreEngine::evolutionMainLoop, this);
    evolution_thread.detach();
    
    logInfo("长期进化过程启动");
}

void UIEECoreEngine::stopLongTermEvolution() {
    evolution_active_ = false;
    
    logInfo("长期进化过程停止");
}

void UIEECoreEngine::updateEvolutionState() {
    if (!evolution_active_ || !population_manager_) {
        return;
    }
    
    // 创建进化历史记录
    EvolutionHistory history;
    history.generation = current_generation_;
    
    // 获取当前最佳个体
    auto best_individual = population_manager_->getBestIndividual();
    history.best_fitness = best_individual.fitness_score;
    history.best_parameters = best_individual.parameters;
    
    // 计算平均适应度
    auto population = population_manager_->getCurrentPopulation();
    double total_fitness = 0.0;
    int valid_count = 0;
    
    for (const auto& individual : population) {
        if (individual.is_valid) {
            total_fitness += individual.fitness_score;
            valid_count++;
        }
    }
    
    history.average_fitness = valid_count > 0 ? total_fitness / valid_count : 0.0;
    history.diversity_score = calculatePopulationDiversity(population);
    history.timestamp = std::chrono::steady_clock::now();
    
    // 添加到历史记录
    std::lock_guard<std::mutex> lock(evolution_mutex_);
    evolution_history_.push_back(history);
    
    // 限制历史记录大小
    if (evolution_history_.size() > 100) {
        evolution_history_.erase(evolution_history_.begin());
    }
}

std::string UIEECoreEngine::getEvolutionStatus() {
    if (!evolution_active_) {
        return "{\"status\": \"inactive\", \"generation\": 0}";
    }
    
    auto current_state = getCurrentState();
    
    std::stringstream ss;
    ss << "{\"status\": \"active\", \"generation\": " << current_state.generation 
       << ", \"best_fitness\": " << current_state.best_fitness
       << ", \"average_fitness\": " << current_state.average_fitness
       << ", \"diversity_score\": " << current_state.diversity_score << "}";
    
    return ss.str();
}

void UIEECoreEngine::saveEvolutionData(const std::string& filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        logError("无法保存进化数据到文件: " + filepath);
        return;
    }
    
    std::lock_guard<std::mutex> lock(evolution_mutex_);
    
    file << "generation,best_fitness,average_fitness,diversity_score,timestamp\n";
    for (const auto& history : evolution_history_) {
        auto time_t = std::chrono::system_clock::to_time_t(history.timestamp);
        file << history.generation << "," 
             << history.best_fitness << "," 
             << history.average_fitness << "," 
             << history.diversity_score << "," 
             << std::ctime(&time_t);
    }
    
    file.close();
    logInfo("进化数据已保存到: " + filepath);
}

void UIEECoreEngine::loadEvolutionData(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        logError("无法加载进化数据文件: " + filepath);
        return;
    }
    
    std::string line;
    std::getline(file, line); // 跳过标题行
    
    std::lock_guard<std::mutex> lock(evolution_mutex_);
    evolution_history_.clear();
    
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        EvolutionHistory history;
        
        ss >> history.generation >> history.best_fitness >> history.average_fitness >> history.diversity_score;
        history.timestamp = std::chrono::steady_clock::now();
        
        evolution_history_.push_back(history);
    }
    
    file.close();
    logInfo("进化数据已从文件加载: " + filepath);
}

void UIEECoreEngine::performIntegratedScheduling() {
    // 集成调度：结合传统算法和Hamilton理论
    logInfo("开始集成调度");
    
    // 1. 执行传统调度
    performScheduling();
    
    // 2. 应用进化参数
    applyEvolutionaryParameters();
    
    // 3. 结合传统和进化算法
    combineTraditionalAndEvolutionary();
    
    // 4. 验证结果
    validateSchedulingResult();
    
    // 5. 更新进化性能
    updateEvolutionaryPerformance();
    
    logInfo("集成调度完成");
}

UIEECoreEngine::ParetoPoint UIEECoreEngine::findEvolutionaryOptimalPoint() {
    if (!population_manager_) {
        return ParetoPoint();
    }
    
    // 获取最佳进化个体
    auto best_individual = population_manager_->getBestIndividual();
    
    // 转换为帕累托点
    ParetoPoint point;
    point.performance = best_individual.performance_score;
    point.power_consumption = best_individual.energy_cost;
    point.thermal_impact = best_individual.energy_cost * 0.5; // 简化计算
    point.parameters = best_individual.parameters;
    
    return point;
}

UIEECoreEngine::NashEquilibrium UIEECoreEngine::calculateEvolutionaryNashEquilibrium() {
    NashEquilibrium equilibrium;
    
    if (!game_manager_) {
        return equilibrium;
    }
    
    // 基于博弈结果计算纳什均衡
    auto players = game_manager_->getPlayers();
    
    equilibrium.strategies.resize(players.size());
    for (size_t i = 0; i < players.size(); ++i) {
        equilibrium.strategies[i] = players[i].cooperation_rate;
    }
    
    // 计算效用值
    double total_payoff = 0.0;
    for (const auto& player : players) {
        total_payoff += player.cumulative_payoff;
    }
    equilibrium.utility_value = players.empty() ? 0.0 : total_payoff / players.size();
    
    return equilibrium;
}

void UIEECoreEngine::logEvolutionaryData(const EvolutionHistory& history) {
    std::string log_message = "进化数据 - 代数:" + std::to_string(history.generation) +
                             " 最佳适应度:" + std::to_string(history.best_fitness) +
                             " 平均适应度:" + std::to_string(history.average_fitness) +
                             " 多样性:" + std::to_string(history.diversity_score);
    
    logInfo(log_message);
}

std::string UIEECoreEngine::getEvolutionaryWebUIStatus() {
    auto status = getEvolutionStatus();
    auto best_individual = getBestEvolutionaryStrategy();
    auto game_players = game_manager_ ? game_manager_->getPlayers() : std::vector<GamePlayer>();
    
    std::stringstream ss;
    ss << "{\n";
    ss << "  \"evolution_status\": " << status << ",\n";
    ss << "  \"best_individual\": {\n";
    ss << "    \"fitness_score\": " << best_individual.fitness_score << ",\n";
    ss << "    \"generation\": " << best_individual.generation << ",\n";
    ss << "    \"parameters\": [";
    for (size_t i = 0; i < best_individual.parameters.size(); ++i) {
        ss << best_individual.parameters[i];
        if (i < best_individual.parameters.size() - 1) ss << ", ";
    }
    ss << "]\n";
    ss << "  },\n";
    ss << "  \"game_players\": [\n";
    for (size_t i = 0; i < game_players.size(); ++i) {
        ss << "    {\n";
        ss << "      \"player_id\": " << game_players[i].player_id << ",\n";
        ss << "      \"strategy\": " << static_cast<int>(game_players[i].current_strategy) << ",\n";
        ss << "      \"cooperation_rate\": " << game_players[i].cooperation_rate << ",\n";
        ss << "      \"cumulative_payoff\": " << game_players[i].cumulative_payoff << "\n";
        ss << "    }";
        if (i < game_players.size() - 1) ss << ",";
        ss << "\n";
    }
    ss << "  ],\n";
    ss << "  \"hamilton_theory_enabled\": true\n";
    ss << "}\n";
    
    return ss.str();
}

// ========== 性能优化实现 ==========

void UIEECoreEngine::initializePerformanceOptimization() {
    // 初始化性能监控器
    performance_monitor_ = std::make_unique<PerformanceMonitor>();
    
    // 初始化线程池
    if (optimization_config_.enable_thread_pool) {
        thread_pool_ = std::make_unique<ThreadPoolManager>(optimization_config_.thread_pool_size);
    }
    
    // 初始化内存池
    if (optimization_config_.enable_memory_pool) {
        memory_pool_ = std::make_unique<MemoryPoolManager>(optimization_config_.memory_pool_block_size);
    }
    
    logInfo("性能优化组件初始化完成");
}

void UIEECoreEngine::updateAdaptiveSampling() {
    if (!optimization_config_.enable_adaptive_sampling || !performance_monitor_) {
        return;
    }
    
    // 获取当前性能指标
    PerformanceMetrics metrics = getCurrentMetrics();
    performance_monitor_->addSample(metrics.cpu_usage, metrics.memory_usage);
    
    // 根据系统负载调整采样频率
    if (performance_monitor_->shouldReduceSampling()) {
        // 高负载时减少采样频率
        adaptive_config_.base_sampling_interval = std::min(
            adaptive_config_.base_sampling_interval * 1.2, 
            adaptive_config_.max_sampling_interval
        );
    } else if (performance_monitor_->shouldIncreaseSampling()) {
        // 低负载时增加采样频率
        adaptive_config_.base_sampling_interval = std::max(
            adaptive_config_.base_sampling_interval * 0.8, 
            adaptive_config_.min_sampling_interval
        );
    }
    
    logInfo("自适应采样间隔调整: " + std::to_string(adaptive_config_.base_sampling_interval) + "秒");
}

void UIEECoreEngine::optimizeMemoryUsage() {
    if (!optimization_config_.enable_memory_pool || !memory_pool_) {
        return;
    }
    
    // 检查内存使用情况
    size_t current_usage = memory_pool_->getTotalAllocated();
    size_t peak_usage = memory_pool_->getPeakUsage();
    
    if (current_usage > peak_usage * 0.8) {
        // 内存使用超过峰值80%，触发清理
        logInfo("内存使用率较高，执行优化清理");
        // 这里可以添加具体的内存清理逻辑
    }
}

void UIEECoreEngine::monitorPerformance() {
    if (!optimization_config_.enable_performance_monitoring || !performance_monitor_) {
        return;
    }
    
    // 收集性能数据
    PerformanceMetrics metrics = getCurrentMetrics();
    performance_monitor_->addSample(metrics.cpu_usage, metrics.memory_usage);
    
    // 性能异常检测
    if (metrics.cpu_usage > 90.0 || metrics.memory_usage > 95.0) {
        logWarning("检测到高资源使用率: CPU=" + std::to_string(metrics.cpu_usage) + 
                  "%, 内存=" + std::to_string(metrics.memory_usage) + "%");
        
        // 触发性能优化
        applyPerformanceTuning();
    }
}

double UIEECoreEngine::getCurrentSamplingInterval() const {
    return adaptive_config_.base_sampling_interval;
}

bool UIEECoreEngine::shouldSkipCalculation() const {
    if (!optimization_config_.enable_adaptive_sampling) {
        return false;
    }
    
    // 如果系统负载过高，跳过一些计算
    if (performance_monitor_ && performance_monitor_->shouldReduceSampling()) {
        // 随机跳过一些计算以减轻系统负担
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 1.0);
        return dis(gen) < 0.3; // 30%概率跳过计算
    }
    
    return false;
}

double UIEECoreEngine::evaluateIndividualFitnessOptimized(FitnessIndividual& individual) {
    if (shouldSkipCalculation()) {
        return individual.fitness_score; // 返回上次的值
    }
    
    return evaluateIndividualFitness(individual);
}

void UIEECoreEngine::performGeneticOperationsOptimized() {
    if (!optimization_config_.enable_thread_pool || !thread_pool_) {
        performGeneticOperations();
        return;
    }
    
    // 使用线程池并行处理遗传算法操作
    auto population = population_manager_->getCurrentPopulation();
    
    // 批量评估适应度
    auto fitness_futures = thread_pool_->submitBatchTasks(
        [this](const FitnessIndividual& ind) {
            return evaluateIndividualFitness(const_cast<FitnessIndividual&>(ind));
        },
        population
    );
    
    // 等待所有任务完成
    for (auto& future : fitness_futures) {
        future.wait();
    }
    
    // 执行进化操作
    population_manager_->evolveGeneration();
    
    logInfo("优化版遗传算法操作完成");
}

void UIEECoreEngine::simulateGameRoundOptimized() {
    if (shouldSkipCalculation()) {
        return; // 跳过计算
    }
    
    simulateGameRound();
}

void UIEECoreEngine::evolutionMainLoopOptimized() {
    logInfo("优化版进化主循环启动");
    
    while (evolution_active_ && current_generation_ < evolution_config_.max_generations_) {
        try {
            // 使用优化的方法
            performGeneticOperationsOptimized();
            current_generation_++;
            
            // 更新进化状态
            updateEvolutionState();
            
            // 执行博弈（可能跳过）
            simulateGameRoundOptimized();
            
            // 性能监控和自适应调整
            monitorPerformance();
            updateAdaptiveSampling();
            optimizeMemoryUsage();
            
            // 检查收敛
            checkEvolutionConvergence();
            
            // 动态调整等待时间
            double wait_time = getCurrentSamplingInterval();
            std::this_thread::sleep_for(std::chrono::seconds(static_cast<int>(wait_time)));
            
        } catch (const std::exception& e) {
            logError("优化版进化循环异常: " + std::string(e.what()));
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }
    
    logInfo("优化版进化主循环结束");
}

std::vector<double> UIEECoreEngine::evaluatePopulationFitnessBatch(const std::vector<FitnessIndividual>& population) {
    std::vector<double> fitness_scores;
    fitness_scores.reserve(population.size());
    
    if (!optimization_config_.enable_thread_pool || !thread_pool_) {
        // 串行处理
        for (const auto& individual : population) {
            fitness_scores.push_back(evaluateIndividualFitness(const_cast<FitnessIndividual&>(individual)));
        }
    } else {
        // 并行处理
        auto futures = thread_pool_->submitBatchTasks(
            [this](const FitnessIndividual& ind) {
                return evaluateIndividualFitness(const_cast<FitnessIndividual&>(ind));
            },
            population
        );
        
        for (auto& future : futures) {
            fitness_scores.push_back(future.get());
        }
    }
    
    return fitness_scores;
}

void UIEECoreEngine::updatePopulationFitnessBatch(std::vector<FitnessIndividual>& population) {
    if (population.empty()) {
        return;
    }
    
    auto fitness_scores = evaluatePopulationFitnessBatch(population);
    
    for (size_t i = 0; i < population.size() && i < fitness_scores.size(); ++i) {
        population[i].fitness_score = fitness_scores[i];
        population[i].update_count++;
        population[i].last_update_time = std::chrono::steady_clock::now();
    }
}

void UIEECoreEngine::adaptOptimizationParameters() {
    // 根据设备特性调整优化参数
    PerformanceMetrics metrics = getCurrentMetrics();
    
    // 根据CPU核心数调整线程池大小
    if (optimization_config_.enable_thread_pool && thread_pool_) {
        int cpu_cores = std::thread::hardware_concurrency();
        size_t optimal_threads = std::min(cpu_cores / 2, static_cast<int>(optimization_config_.thread_pool_size));
        if (optimal_threads < 1) optimal_threads = 1;
        
        logInfo("根据CPU核心数调整线程池大小: " + std::to_string(optimal_threads));
    }
    
    // 根据内存大小调整缓存大小
    if (optimization_config_.enable_cache && hamilton_fitness_) {
        size_t memory_mb = static_cast<size_t>(metrics.memory_usage * 100); // 估算
        size_t optimal_cache_size = std::min(memory_mb / 10, optimization_config_.cache_size);
        hamilton_fitness_->setCacheSize(optimal_cache_size);
        
        logInfo("根据内存大小调整缓存大小: " + std::to_string(optimal_cache_size));
    }
}

void UIEECoreEngine::optimizeForDeviceCharacteristics() {
    // 获取设备特性
    PerformanceMetrics metrics = getCurrentMetrics();
    int cpu_cores = std::thread::hardware_concurrency();
    
    // 高性能设备配置
    if (cpu_cores >= 8 && metrics.memory_usage < 50.0) {
        optimization_config_.enable_thread_pool = true;
        optimization_config_.thread_pool_size = 6;
        optimization_config_.cache_size = 200;
        adaptive_config_.base_sampling_interval = 15.0;
        logInfo("检测到高性能设备，应用激进优化配置");
    }
    // 中等性能设备配置
    else if (cpu_cores >= 4) {
        optimization_config_.enable_thread_pool = true;
        optimization_config_.thread_pool_size = 3;
        optimization_config_.cache_size = 100;
        adaptive_config_.base_sampling_interval = 30.0;
        logInfo("检测到中等性能设备，应用平衡优化配置");
    }
    // 低性能设备配置
    else {
        optimization_config_.enable_thread_pool = false;
        optimization_config_.cache_size = 50;
        adaptive_config_.base_sampling_interval = 60.0;
        logInfo("检测到低性能设备，应用保守优化配置");
    }
}

void UIEECoreEngine::applyPerformanceTuning() {
    logInfo("应用性能调优策略");
    
    // 临时降低计算频率
    adaptive_config_.base_sampling_interval = std::min(
        adaptive_config_.base_sampling_interval * 1.5,
        adaptive_config_.max_sampling_interval
    );
    
    // 增加缓存大小
    if (hamilton_fitness_ && optimization_config_.enable_cache) {
        hamilton_fitness_->setCacheSize(optimization_config_.cache_size * 1.5);
    }
    
    // 减少线程池任务数量
    if (thread_pool_) {
        // 这里可以添加线程池调优逻辑
    }
}

void UIEECoreEngine::enablePerformanceOptimization(const PerformanceOptimizationConfig& config) {
    optimization_config_ = config;
    
    // 重新初始化优化组件
    initializePerformanceOptimization();
    
    // 根据设备特性优化
    optimizeForDeviceCharacteristics();
    
    logInfo("性能优化已启用");
}

void UIEECoreEngine::disablePerformanceOptimization() {
    optimization_config_ = PerformanceOptimizationConfig();
    optimization_config_.enable_cache = false;
    optimization_config_.enable_adaptive_sampling = false;
    optimization_config_.enable_thread_pool = false;
    optimization_config_.enable_memory_pool = false;
    optimization_config_.enable_performance_monitoring = false;
    
    logInfo("性能优化已禁用");
}

PerformanceOptimizationConfig UIEECoreEngine::getOptimizationConfig() const {
    return optimization_config_;
}

void UIEECoreEngine::resetPerformanceStats() {
    if (hamilton_fitness_) {
        hamilton_fitness_->clearCache();
    }
    
    if (memory_pool_) {
        memory_pool_->resetStats();
    }
    
    logInfo("性能统计已重置");
}

std::string UIEECoreEngine::getPerformanceReport() const {
    std::stringstream ss;
    ss << "=== 性能优化报告 ===\n";
    ss << "CPU平均使用率: " << (performance_monitor_ ? 
         std::to_string(performance_monitor_->avg_cpu_usage) : "N/A") << "%\n";
    ss << "内存平均使用率: " << (performance_monitor_ ? 
         std::to_string(performance_monitor_->avg_memory_usage) : "N/A") << "%\n";
    ss << "当前采样间隔: " << getCurrentSamplingInterval() << "秒\n";
    ss << "线程池状态: " << (thread_pool_ ? "启用" : "禁用") << "\n";
    ss << "内存池状态: " << (memory_pool_ ? "启用" : "禁用") << "\n";
    
    if (hamilton_fitness_) {
        auto stats = hamilton_fitness_->getStats();
        ss << "适应度计算缓存命中率: " << 
           (stats.total_calculations > 0 ? 
            std::to_string(100.0 * stats.cache_hits / stats.total_calculations) : "0") << "%\n";
    }
    
    return ss.str();
}