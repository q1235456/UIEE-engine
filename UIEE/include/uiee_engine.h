#ifndef UIEE_ENGINE_H
#define UIEE_ENGINE_H

#include <string>
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <random>
#include <memory>

// UIEE核心引擎类
class UIEECoreEngine {
public:
    // 构造函数
    UIEECoreEngine();
    ~UIEECoreEngine();
    
    // 启动和停止引擎
    bool start();
    void stop();
    
    // 配置管理
    void loadConfig(const std::string& configPath);
    void saveConfig(const std::string& configPath);
    
    // 性能监控
    struct PerformanceMetrics {
        double cpu_usage;
        double memory_usage;
        double gpu_usage;
        double thermal_state;
        double battery_level;
        double responsiveness_score;
        double fluency_score;
        double efficiency_score;
        double ces_score; // 综合体验分数
    };
    
    PerformanceMetrics getCurrentMetrics();
    
    // 任务管理
    struct TaskInfo {
        std::string name;
        int pid;
        int priority;
        std::string app_type; // game, social, media, productivity
        double cpu_affinity;
        bool is_foreground;
        std::chrono::steady_clock::time_point start_time;
    };
    
    void addTask(const TaskInfo& task);
    void removeTask(int pid);
    std::vector<TaskInfo> getActiveTasks();
    
    // 场景感知
    enum SceneType {
        SCENE_GAME,
        SCENE_SOCIAL,
        SCENE_MEDIA,
        SCENE_PRODUCTIVITY,
        SCENE_UNKNOWN
    };
    
    SceneType detectCurrentScene();
    void setScenePreference(SceneType scene);
    
    // 帕累托最优算法
    struct ParetoPoint {
        double performance;
        double power_consumption;
        double thermal_impact;
        std::vector<double> parameters;
    };
    
    std::vector<ParetoPoint> calculateParetoFrontier(const std::vector<ParetoPoint>& points);
    ParetoPoint findOptimalPoint(const std::vector<ParetoPoint>& frontier);
    
    // 纳什均衡算法
    struct NashEquilibrium {
        std::vector<double> strategies;
        double utility_value;
    };
    
    NashEquilibrium calculateNashEquilibrium(const std::vector<std::vector<double>>& payoff_matrix);
    
    // CTO集成
    struct CTOConfig {
        bool enable_task_binding;
        bool enable_io_scheduling;
        bool enable_cpu_affinity;
        int max_bound_cores;
    };
    
    void applyCTOConfig(const CTOConfig& config);
    void bindTaskToCore(int pid, int core_id);
    
    // ========== Hamilton适应度理论落地实现 ==========
    
    // 性能优化相关成员变量
    std::unique_ptr<PerformanceMonitor> performance_monitor_;
    AdaptiveSamplingConfig adaptive_config_;
    PerformanceOptimizationConfig optimization_config_;
    std::unique_ptr<ThreadPoolManager> thread_pool_;
    std::unique_ptr<MemoryPoolManager> memory_pool_;
    
    // 性能优化方法
    void initializePerformanceOptimization();
    void updateAdaptiveSampling();
    void optimizeMemoryUsage();
    void monitorPerformance();
    double getCurrentSamplingInterval() const;
    bool shouldSkipCalculation() const;
    
    // 性能优化相关公共接口
    void enablePerformanceOptimization(const PerformanceOptimizationConfig& config);
    void disablePerformanceOptimization();
    PerformanceOptimizationConfig getOptimizationConfig() const;
    void resetPerformanceStats();
    std::string getPerformanceReport() const;
    
    // 性能优化：适应度缓存
    struct FitnessCache {
        PerformanceMetrics metrics;
        double cached_fitness;
        std::chrono::steady_clock::time_point cache_time;
        bool is_valid;
        size_t hash_value;  // 指标哈希值，用于快速比较
        
        FitnessCache() : cached_fitness(0.0), is_valid(false), hash_value(0) {}
        
        size_t calculateHash() const {
            // 简单的哈希计算，用于快速比较指标变化
            size_t hash = 1469598103934665603ULL; // FNV-1a 初始值
            const char* data = reinterpret_cast<const char*>(&metrics);
            for (size_t i = 0; i < sizeof(PerformanceMetrics); ++i) {
                hash ^= data[i];
                hash *= 1099511628211ULL;
            }
            return hash;
        }
    };
    
    // 性能监控器
    struct PerformanceMonitor {
        std::chrono::steady_clock::time_point last_check_time;
        double cpu_usage_samples[10];  // CPU使用率历史样本
        double memory_usage_samples[10];  // 内存使用率历史样本
        int sample_index;
        double avg_cpu_usage;
        double avg_memory_usage;
        bool is_high_performance_mode;
        
        PerformanceMonitor() : last_check_time(std::chrono::steady_clock::now()),
                              sample_index(0), avg_cpu_usage(0.0), 
                              avg_memory_usage(0.0), is_high_performance_mode(false) {
            std::fill(std::begin(cpu_usage_samples), std::end(cpu_usage_samples), 0.0);
            std::fill(std::begin(memory_usage_samples), std::end(memory_usage_samples), 0.0);
        }
        
        void addSample(double cpu, double memory) {
            cpu_usage_samples[sample_index] = cpu;
            memory_usage_samples[sample_index] = memory;
            sample_index = (sample_index + 1) % 10;
            
            // 计算平均值
            double cpu_sum = 0.0, mem_sum = 0.0;
            for (int i = 0; i < 10; ++i) {
                cpu_sum += cpu_usage_samples[i];
                mem_sum += memory_usage_samples[i];
            }
            avg_cpu_usage = cpu_sum / 10.0;
            avg_memory_usage = mem_sum / 10.0;
        }
        
        bool shouldReduceSampling() const {
            return avg_cpu_usage > 80.0 || avg_memory_usage > 85.0;
        }
        
        bool shouldIncreaseSampling() const {
            return avg_cpu_usage < 20.0 && avg_memory_usage < 30.0;
        }
    };
    
    // 适应度个体 - 表示一个调度策略（性能优化版）
    struct FitnessIndividual {
        std::vector<double> parameters;  // 调度参数
        double fitness_score;            // 适应度分数
        double performance_score;        // 性能分数
        double efficiency_score;         // 效率分数
        double energy_cost;              // 能量代价
        std::chrono::steady_clock::time_point creation_time;
        std::chrono::steady_clock::time_point last_update_time;
        int generation;                  // 所属代数
        bool is_valid;                   // 是否有效
        int update_count;                // 更新次数（用于自适应调整）
        
        FitnessIndividual() : fitness_score(0.0), performance_score(0.0), 
                             efficiency_score(0.0), energy_cost(0.0), 
                             generation(0), is_valid(true), update_count(0) {}
    };
    
    // 自适应采样配置
    struct AdaptiveSamplingConfig {
        double base_sampling_interval;   // 基础采样间隔（秒）
        double min_sampling_interval;    // 最小采样间隔
        double max_sampling_interval;    // 最大采样间隔
        double cpu_threshold_high;       // CPU高负载阈值
        double cpu_threshold_low;        // CPU低负载阈值
        double memory_threshold_high;    // 内存高负载阈值
        double memory_threshold_low;     // 内存低负载阈值
        int adaptation_window;           // 自适应窗口大小
        
        AdaptiveSamplingConfig() : 
            base_sampling_interval(30.0), min_sampling_interval(5.0), 
            max_sampling_interval(120.0), cpu_threshold_high(80.0), 
            cpu_threshold_low(20.0), memory_threshold_high(85.0), 
            memory_threshold_low(30.0), adaptation_window(10) {}
    };
    
    // 线程池管理器（性能优化）
    class ThreadPoolManager {
    public:
        ThreadPoolManager(size_t num_threads = 4);
        ~ThreadPoolManager();
        
        // 任务提交
        template<typename Func, typename... Args>
        auto submitTask(Func&& func, Args&&... args) 
            -> std::future<typename std::result_of<Func(Args...)>::type>;
        
        // 批量任务提交
        template<typename Func, typename... Args>
        auto submitBatchTasks(std::vector<Func> funcs, Args&&... args) 
            -> std::vector<std::future<typename std::result_of<Func(Args...)>::type>>;
        
        // 线程池控制
        void shutdown();
        bool isShutdown() const;
        size_t getActiveTasks() const;
        size_t getTotalTasks() const;
        
    private:
        class Impl;
        std::unique_ptr<Impl> impl_;
    };
    
    // 内存池管理器（性能优化）
    class MemoryPoolManager {
    public:
        MemoryPoolManager(size_t block_size = 1024, size_t max_blocks = 1000);
        ~MemoryPoolManager();
        
        // 内存分配/释放
        void* allocate(size_t size);
        void deallocate(void* ptr);
        
        // 统计信息
        size_t getTotalAllocated() const;
        size_t getPeakUsage() const;
        size_t getActiveBlocks() const;
        void resetStats();
        
    private:
        class Impl;
        std::unique_ptr<Impl> impl_;
    };
    
    // 性能优化配置
    struct PerformanceOptimizationConfig {
        bool enable_cache;
        bool enable_adaptive_sampling;
        bool enable_thread_pool;
        bool enable_memory_pool;
        bool enable_performance_monitoring;
        size_t cache_size;
        size_t thread_pool_size;
        size_t memory_pool_block_size;
        double performance_threshold;
        
        PerformanceOptimizationConfig() :
            enable_cache(true), enable_adaptive_sampling(true),
            enable_thread_pool(true), enable_memory_pool(true),
            enable_performance_monitoring(true), cache_size(100),
            thread_pool_size(4), memory_pool_block_size(1024),
            performance_threshold(0.1) {}
    };
    
    // Hamilton适应度函数（性能优化版）
    class HamiltonFitnessFunction {
    public:
        HamiltonFitnessFunction();
        ~HamiltonFitnessFunction();
        
        // 核心计算方法
        double calculateFitness(const PerformanceMetrics& metrics, const std::vector<double>& parameters);
        double calculatePerformanceComponent(const PerformanceMetrics& metrics);
        double calculateEfficiencyComponent(const PerformanceMetrics& metrics);
        double calculateEnergyCost(const PerformanceMetrics& metrics);
        
        // 缓存管理
        void clearCache();
        void setCacheSize(size_t size);
        size_t getCacheHits() const { return cache_hits_; }
        size_t getCacheMisses() const { return cache_misses_; }
        
        // 自适应权重调整
        void updateAdaptiveWeights(const PerformanceMetrics& metrics);
        void setWeights(double alpha, double beta, double gamma);
        
        // 性能统计
        struct PerformanceStats {
            size_t total_calculations;
            size_t cache_hits;
            size_t cache_misses;
            double avg_calculation_time_ms;
            std::chrono::steady_clock::time_point last_reset;
            
            PerformanceStats() : total_calculations(0), cache_hits(0), 
                               cache_misses(0), avg_calculation_time_ms(0.0),
                               last_reset(std::chrono::steady_clock::now()) {}
        };
        
        PerformanceStats getStats() const;
        
    private:
        double alpha_;  // 性能权重
        double beta_;   // 效率权重
        double gamma_;  // 代价权重
        
        // 缓存系统
        std::unique_ptr<FitnessCache[]> cache_;
        size_t cache_size_;
        size_t cache_index_;
        size_t cache_hits_;
        size_t cache_misses_;
        
        // 性能统计
        mutable std::mutex stats_mutex_;
        PerformanceStats stats_;
        
        // 内部辅助方法
        size_t findCacheEntry(const PerformanceMetrics& metrics) const;
        bool isCacheValid(const FitnessCache& cache) const;
        void updateStats(double calculation_time_ms) const;
    };
    
    // 种群进化管理器
    class PopulationEvolutionManager {
    public:
        PopulationEvolutionManager(size_t population_size = 50);
        void initializePopulation();
        void evolveGeneration();
        FitnessIndividual getBestIndividual();
        std::vector<FitnessIndividual> getCurrentPopulation();
        void setFitnessFunction(std::shared_ptr<HamiltonFitnessFunction> fitness_func);
        
    private:
        size_t population_size_;
        std::vector<FitnessIndividual> population_;
        int current_generation_;
        std::shared_ptr<HamiltonFitnessFunction> fitness_function_;
        std::mt19937 rng_;
        
        // 遗传算法操作
        FitnessIndividual crossover(const FitnessIndividual& parent1, const FitnessIndividual& parent2);
        void mutate(FitnessIndividual& individual);
        std::vector<FitnessIndividual> selectParents();
        bool shouldTerminate();
    };
    
    // ========== 连续囚徒困境博弈学习 ==========
    
    // 博弈策略
    enum GameStrategy {
        STRATEGY_COOPERATE,      // 合作
        STRATEGY_DEFECT,         // 背叛
        STRATEGY_TIT_FOR_TAT,    // 以牙还牙
        STRATEGY_GENEROUS,       // 宽容
        STRATEGY_ADAPTIVE        // 自适应
    };
    
    // 博弈参与者
    struct GamePlayer {
        int player_id;
        GameStrategy current_strategy;
        std::vector<bool> action_history;  // 行动历史
        std::vector<double> payoff_history; // 收益历史
        double cumulative_payoff;
        double cooperation_rate;
        
        GamePlayer(int id) : player_id(id), current_strategy(STRATEGY_COOPERATE),
                           cumulative_payoff(0.0), cooperation_rate(0.0) {}
    };
    
    // 连续囚徒困境管理器
    class RepeatedPrisonersDilemma {
    public:
        RepeatedPrisonersDilemma();
        void addPlayer(const GamePlayer& player);
        void simulateRound();
        void updateStrategies();
        double getPayoff(GameStrategy strategy1, GameStrategy strategy2);
        std::vector<GamePlayer> getPlayers();
        void resetGame();
        
    private:
        std::vector<GamePlayer> players_;
        int current_round_;
        double cooperation_reward_;
        double defection_reward_;
        double mutual_punishment_;
        double temptation_;
        
        // 策略更新算法
        void updateAdaptiveStrategy(GamePlayer& player);
        double calculateExpectedPayoff(const GamePlayer& player, GameStrategy strategy);
    };
    
    // ========== 长期自我迭代进化框架 ==========
    
    // 进化历史记录
    struct EvolutionHistory {
        int generation;
        double best_fitness;
        double average_fitness;
        double diversity_score;
        std::chrono::steady_clock::time_point timestamp;
        std::vector<double> best_parameters;
    };
    
    // 长期进化管理器
    class LongTermEvolutionManager {
    public:
        LongTermEvolutionManager();
        void startEvolution();
        void stopEvolution();
        void updateEvolutionState();
        EvolutionHistory getCurrentState();
        std::vector<EvolutionHistory> getEvolutionHistory();
        void saveEvolutionData(const std::string& filepath);
        void loadEvolutionData(const std::string& filepath);
        
    private:
        std::atomic<bool> evolution_running_;
        std::thread evolution_thread_;
        std::vector<EvolutionHistory> evolution_history_;
        std::mutex evolution_mutex_;
        
        PopulationEvolutionManager population_manager_;
        RepeatedPrisonersDilemma game_manager_;
        HamiltonFitnessFunction fitness_function_;
        
        // 进化参数
        int max_generations_;
        int current_generation_;
        double convergence_threshold_;
        std::chrono::seconds evolution_interval_;
    };
    
    // 调度算法
    void performScheduling();
    
    // ========== Hamilton理论公共接口 ==========
    
    // 适应度理论接口
    double calculateHamiltonFitness(const PerformanceMetrics& metrics, const std::vector<double>& parameters);
    void initializeEvolutionPopulation(size_t population_size);
    void evolvePopulation();
    FitnessIndividual getBestEvolutionaryStrategy();
    void setEvolutionParameters(double alpha, double beta, double gamma);
    
    // 连续囚徒困境接口
    void startRepeatedGame();
    void stopRepeatedGame();
    void addGamePlayer(int player_id, GameStrategy strategy);
    void updateGameStrategies();
    double getGamePayoff(int player_id);
    double getCooperationRate();
    
    // 长期进化接口
    void startLongTermEvolution();
    void stopLongTermEvolution();
    void updateEvolutionState();
    std::string getEvolutionStatus();
    void saveEvolutionData(const std::string& filepath);
    void loadEvolutionData(const std::string& filepath);
    
    // 集成调度 - 结合传统算法和Hamilton理论
    void performIntegratedScheduling();
    ParetoPoint findEvolutionaryOptimalPoint();
    NashEquilibrium calculateEvolutionaryNashEquilibrium();
    
    // 日志管理
    void logInfo(const std::string& message);
    void logError(const std::string& message);
    void logWarning(const std::string& message);
    void logPerformance(const PerformanceMetrics& metrics);
    void logEvolutionaryData(const EvolutionHistory& history);
    
    // Web UI接口
    std::string getWebUIStatus();
    void updateWebUIConfig(const std::string& json_config);
    std::string getEvolutionaryWebUIStatus();
    
private:
    // 内部成员
    std::atomic<bool> running_;
    std::thread main_thread_;
    std::thread monitor_thread_;
    std::mutex tasks_mutex_;
    std::mutex config_mutex_;
    std::condition_variable cv_;
    
    // 配置
    struct Config {
        bool enable_engine = true;
        int scheduling_interval = 5;
        bool optimization_enabled = true;
        double responsiveness_weight = 0.3;
        double fluency_weight = 0.3;
        double efficiency_weight = 0.2;
        double thermal_weight = 0.2;
        SceneType current_scene = SCENE_UNKNOWN;
        CTOConfig cto_config;
    } config_;
    
    // 任务列表
    std::vector<TaskInfo> active_tasks_;
    
    // 性能历史数据
    std::vector<PerformanceMetrics> performance_history_;
    static constexpr size_t MAX_HISTORY_SIZE = 1000;
    
    // 设备信息
    struct DeviceInfo {
        int cpu_cores;
        std::string soc_model;
        double base_frequency;
        std::vector<double> core_frequencies;
    } device_info_;
    
    // （重复的 Hamilton 理论成员变量块已移除，相关成员在上方已声明）
    
    // 私有方法
    void mainLoop();
    void monitoringLoop();
    void initializeDeviceInfo();
    double calculateCES(const PerformanceMetrics& metrics);
    void updateTaskPriorities();
    void applySchedulingPolicies();
    std::string getCurrentTimestamp();
    
    // ========== Hamilton理论私有实现方法 ==========
    
    // 适应度理论私有方法
    void initializeHamiltonComponents();
    void updateFitnessParameters();
    double evaluateIndividualFitness(FitnessIndividual& individual);
    void performGeneticOperations();
    void updatePopulationDiversity();
    
    // 连续囚徒困境私有方法
    void initializeGameComponents();
    void simulateGameRound();
    void updatePlayerStrategies();
    double calculatePayoffMatrix();
    void analyzeCooperationDynamics();
    
    // 长期进化私有方法
    void evolutionMainLoop();
    void checkEvolutionConvergence();
    void updateEvolutionMetrics();
    void saveEvolutionSnapshot();
    void loadEvolutionCheckpoint();
    
    // 集成调度私有方法
    void combineTraditionalAndEvolutionary();
    void applyEvolutionaryParameters();
    void validateSchedulingResult();
    void updateEvolutionaryPerformance();
    
    // ========== 性能优化私有方法 ==========
    
    // 性能优化核心方法
    void initializePerformanceOptimization();
    void updateAdaptiveSampling();
    void optimizeMemoryUsage();
    void monitorPerformance();
    double getCurrentSamplingInterval() const;
    bool shouldSkipCalculation() const;
    
    // 优化版本的方法
    double evaluateIndividualFitnessOptimized(FitnessIndividual& individual);
    void performGeneticOperationsOptimized();
    void simulateGameRoundOptimized();
    void evolutionMainLoopOptimized();
    
    // 批量处理方法
    std::vector<double> evaluatePopulationFitnessBatch(const std::vector<FitnessIndividual>& population);
    void updatePopulationFitnessBatch(std::vector<FitnessIndividual>& population);
    
    // 自适应优化方法
    void adaptOptimizationParameters();
    void optimizeForDeviceCharacteristics();
    void applyPerformanceTuning();
    
    // 系统调用封装
    double getCPUUsage();
    double getMemoryUsage();
    double getThermalState();
    std::vector<int> getRunningPIDs();
    std::string getProcessName(int pid);
    bool setProcessPriority(int pid, int priority);
    bool setCPUAffinity(int pid, const std::vector<int>& cores);
    
    // ========== Hamilton理论成员变量 ==========
    
    // 适应度理论组件
    std::shared_ptr<HamiltonFitnessFunction> hamilton_fitness_;
    std::shared_ptr<PopulationEvolutionManager> population_manager_;
    
    // 连续囚徒困境组件
    std::shared_ptr<RepeatedPrisonersDilemma> game_manager_;
    std::atomic<bool> game_running_;
    
    // 长期进化组件
    std::shared_ptr<LongTermEvolutionManager> evolution_manager_;
    std::atomic<bool> evolution_active_;
    
    // 进化参数
    struct EvolutionConfig {
        double alpha_weight = 0.4;      // 性能权重
        double beta_weight = 0.3;       // 效率权重
        double gamma_weight = 0.3;      // 代价权重
        size_t population_size = 50;    // 种群大小
        int max_generations = 1000;     // 最大代数
        double mutation_rate = 0.1;     // 变异率
        double crossover_rate = 0.8;    // 交叉率
        double convergence_threshold = 1e-6; // 收敛阈值
    } evolution_config_;
};

#endif // UIEE_ENGINE_H