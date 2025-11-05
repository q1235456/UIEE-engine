#include "uiee_engine.h"
#include <iostream>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>

// 全局引擎实例
std::unique_ptr<UIEECoreEngine> g_engine;

// 信号处理
void signalHandler(int signal) {
    std::cout << "收到信号 " << signal << "，正在关闭UIEE引擎..." << std::endl;
    if (g_engine) {
        g_engine->stop();
    }
    exit(0);
}

void printUsage() {
    std::cout << "UIEE智能调度引擎 v3.0" << std::endl;
    std::cout << "用法: uiee_engine [选项]" << std::endl;
    std::cout << "选项:" << std::endl;
    std::cout << "  -h, --help     显示帮助信息" << std::endl;
    std::cout << "  -c, --config   指定配置文件路径" << std::endl;
    std::cout << "  -d, --daemon   后台守护模式运行" << std::endl;
    std::cout << "  -v, --version  显示版本信息" << std::endl;
    std::cout << "  --status       显示引擎状态" << std::endl;
    std::cout << "  --test         运行测试模式" << std::endl;
}

void printVersion() {
    std::cout << "UIEE智能调度引擎 v3.0.0" << std::endl;
    std::cout << "基于帕累托最优和纳什均衡算法" << std::endl;
    std::cout << "C++原生实现" << std::endl;
    std::cout << "作者: 元昔" << std::endl;
}

void runTest() {
    std::cout << "=== UIEE引擎测试模式 ===" << std::endl;
    
    // 创建测试引擎
    UIEECoreEngine engine;
    
    // 测试配置加载
    std::cout << "1. 测试配置管理..." << std::endl;
    std::string test_config_path;
    if (getenv("MODPATH")) {
        test_config_path = std::string(getenv("MODPATH")) + "/data/config/uiee.conf";
    } else {
        test_config_path = "/data/adb/modules/uiee_smart_engine/data/config/uiee.conf";
    }
    std::cout << "   尝试加载配置文件: " << test_config_path << std::endl;
    
    // 检查文件是否存在
    std::ifstream test_file(test_config_path);
    if (test_file.is_open()) {
        std::cout << "   ✓ 配置文件存在且可读" << std::endl;
        test_file.close();
    } else {
        std::cout << "   ✗ 配置文件不存在或不可读" << std::endl;
    }
    
    engine.loadConfig(test_config_path);
    
    // 测试性能指标获取
    std::cout << "2. 测试性能指标获取..." << std::endl;
    auto metrics = engine.getCurrentMetrics();
    std::cout << "   CPU使用率: " << metrics.cpu_usage << "%" << std::endl;
    std::cout << "   内存使用率: " << metrics.memory_usage << "%" << std::endl;
    std::cout << "   CES分数: " << metrics.ces_score << std::endl;
    
    // 测试任务管理
    std::cout << "3. 测试任务管理..." << std::endl;
    UIEECoreEngine::TaskInfo test_task;
    test_task.name = "test_app";
    test_task.pid = 1234;
    test_task.priority = 5;
    test_task.app_type = "game";
    test_task.is_foreground = true;
    
    engine.addTask(test_task);
    auto tasks = engine.getActiveTasks();
    std::cout << "   活动任务数量: " << tasks.size() << std::endl;
    
    // 测试场景检测
    std::cout << "4. 测试场景检测..." << std::endl;
    auto scene = engine.detectCurrentScene();
    std::cout << "   当前场景: " << static_cast<int>(scene) << std::endl;
    
    // 测试帕累托最优算法
    std::cout << "5. 测试帕累托最优算法..." << std::endl;
    std::vector<UIEECoreEngine::ParetoPoint> test_points;
    for (int i = 0; i < 5; i++) {
        UIEECoreEngine::ParetoPoint point;
        point.performance = 50 + i * 10;
        point.power_consumption = 100 - i * 15;
        point.thermal_impact = 30 + i * 5;
        test_points.push_back(point);
    }
    
    auto pareto_frontier = engine.calculateParetoFrontier(test_points);
    std::cout << "   帕累托前沿点数: " << pareto_frontier.size() << std::endl;
    
    auto optimal = engine.findOptimalPoint(pareto_frontier);
    std::cout << "   最优点性能: " << optimal.performance << std::endl;
    
    // 测试纳什均衡算法
    std::cout << "6. 测试纳什均衡算法..." << std::endl;
    std::vector<std::vector<double>> payoff_matrix = {
        {3, 1},
        {0, 2}
    };
    
    auto equilibrium = engine.calculateNashEquilibrium(payoff_matrix);
    std::cout << "   均衡策略数量: " << equilibrium.strategies.size() << std::endl;
    std::cout << "   均衡效用值: " << equilibrium.utility_value << std::endl;
    
    std::cout << "=== 测试完成 ===" << std::endl;
}

void runStatus() {
    std::cout << "=== UIEE引擎状态 ===" << std::endl;
    
    // 检查引擎是否在运行
    std::ifstream pid_file("/data/adb/modules/uiee_smart_engine/data/engine.pid");
    if (pid_file.is_open()) {
        int pid;
        pid_file >> pid;
        pid_file.close();
        
        if (kill(pid, 0) == 0) {
            std::cout << "引擎状态: 运行中 (PID: " << pid << ")" << std::endl;
        } else {
            std::cout << "引擎状态: 已停止" << std::endl;
        }
    } else {
        std::cout << "引擎状态: 未启动" << std::endl;
    }
    
    // 显示Web UI状态
    std::ifstream web_pid_file("/data/adb/modules/uiee_smart_engine/data/web_ui.pid");
    if (web_pid_file.is_open()) {
        int web_pid;
        web_pid_file >> web_pid;
        web_pid_file.close();
        
        if (kill(web_pid, 0) == 0) {
            std::cout << "Web UI状态: 运行中 (PID: " << web_pid << ")" << std::endl;
        } else {
            std::cout << "Web UI状态: 已停止" << std::endl;
        }
    } else {
        std::cout << "Web UI状态: 未启动" << std::endl;
    }
    
    // 显示日志信息
    std::cout << "\n最近日志:" << std::endl;
    std::ifstream log_file("/data/adb/modules/uiee_smart_engine/logs/engine.log");
    if (log_file.is_open()) {
        std::string line;
        int line_count = 0;
        while (std::getline(log_file, line) && line_count < 5) {
            std::cout << "  " << line << std::endl;
            line_count++;
        }
        log_file.close();
    } else {
        std::cout << "  无日志文件" << std::endl;
    }
}

int main(int argc, char* argv[]) {
    // 设置信号处理
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    // 解析命令行参数
    std::string config_path;
    bool daemon_mode = false;
    bool test_mode = false;
    bool status_mode = false;
    
    // 默认配置路径 - 优先使用data/config目录
    if (getenv("MODPATH")) {
        config_path = std::string(getenv("MODPATH")) + "/data/config/uiee.conf";
    } else {
        config_path = "/data/adb/modules/uiee_smart_engine/data/config/uiee.conf";
    }
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            printUsage();
            return 0;
        } else if (arg == "-v" || arg == "--version") {
            printVersion();
            return 0;
        } else if (arg == "-c" || arg == "--config") {
            if (i + 1 < argc) {
                config_path = argv[++i];
            }
        } else if (arg == "-d" || arg == "--daemon") {
            daemon_mode = true;
        } else if (arg == "--test") {
            test_mode = true;
        } else if (arg == "--status") {
            status_mode = true;
        }
    }
    
    // 处理特殊模式
    if (test_mode) {
        runTest();
        return 0;
    }
    
    if (status_mode) {
        runStatus();
        return 0;
    }
    
    // 守护进程模式
    if (daemon_mode) {
        // 创建守护进程
        pid_t pid = fork();
        if (pid < 0) {
            std::cerr << "创建守护进程失败" << std::endl;
            return 1;
        }
        
        if (pid > 0) {
            // 父进程退出
            return 0;
        }
        
        // 子进程成为守护进程
        setsid();
        umask(0);
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
    }
    
    // 创建引擎实例
    g_engine = std::make_unique<UIEECoreEngine>();
    
    // 加载配置
    g_engine->loadConfig(config_path);
    
    // 启动引擎
    if (!g_engine->start()) {
        std::cerr << "启动UIEE引擎失败" << std::endl;
        return 1;
    }
    
    std::cout << "UIEE智能调度引擎 v3.0 已启动" << std::endl;
    std::cout << "配置路径: " << config_path << std::endl;
    std::cout << "Web UI: http://localhost:8080" << std::endl;
    std::cout << "按 Ctrl+C 停止引擎" << std::endl;
    
    // 保持程序运行
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    return 0;
}