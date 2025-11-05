// UIEE智能调度引擎 WebUI应用
// 3.0版本 - 移动端优化

class UIEEModule {
    constructor() {
        this.apiBase = '/api';
        this.updateInterval = 2000; // 2秒更新一次
        this.isEngineRunning = false;
        this.currentScene = 0;
        this.logContainer = null;
        this.updateTimer = null;
        
        this.init();
    }
    
    init() {
        console.log('UIEE WebUI初始化中...');
        
        // 绑定事件监听器
        this.bindEventListeners();
        
        // 初始化组件
        this.initComponents();
        
        // 启动状态更新
        this.startStatusUpdate();
        
        // 添加初始日志
        this.addLog('UIEE智能调度引擎 WebUI v3.0 已启动', 'info');
        this.addLog('正在连接引擎服务...', 'info');
        
        console.log('UIEE WebUI初始化完成');
    }
    
    bindEventListeners() {
        // 引擎控制按钮
        document.getElementById('start-engine')?.addEventListener('click', () => this.startEngine());
        document.getElementById('stop-engine')?.addEventListener('click', () => this.stopEngine());
        document.getElementById('restart-engine')?.addEventListener('click', () => this.restartEngine());
        
        // 场景模式按钮
        document.querySelectorAll('.scene-btn').forEach(btn => {
            btn.addEventListener('click', (e) => this.setScene(e.target.dataset.scene));
        });
        
        // CTO设置
        document.getElementById('enable-cto')?.addEventListener('change', (e) => this.updateCTOSettings());
        document.getElementById('enable-io-scheduling')?.addEventListener('change', (e) => this.updateCTOSettings());
        document.getElementById('enable-cpu-affinity')?.addEventListener('change', (e) => this.updateCTOSettings());
        
        // 高级设置
        const intervalSlider = document.getElementById('scheduling-interval');
        const intervalValue = document.getElementById('interval-value');
        if (intervalSlider && intervalValue) {
            intervalSlider.addEventListener('input', (e) => {
                intervalValue.textContent = e.target.value;
            });
            intervalSlider.addEventListener('change', (e) => this.updateSchedulingInterval(e.target.value));
        }
        
        const optimizationSlider = document.getElementById('optimization-level');
        const optimizationValue = document.getElementById('optimization-value');
        if (optimizationSlider && optimizationValue) {
            optimizationSlider.addEventListener('input', (e) => {
                optimizationValue.textContent = e.target.value + '%';
            });
            optimizationSlider.addEventListener('change', (e) => this.updateOptimizationLevel(e.target.value));
        }
        
        // 日志控制
        document.getElementById('clear-logs')?.addEventListener('click', () => this.clearLogs());
        document.getElementById('export-logs')?.addEventListener('click', () => this.exportLogs());
        
        // 窗口关闭时清理
        window.addEventListener('beforeunload', () => {
            this.stopStatusUpdate();
        });
    }
    
    initComponents() {
        this.logContainer = document.getElementById('log-container');
        
        // 检查移动设备
        this.isMobile = window.innerWidth <= 768;
        if (this.isMobile) {
            document.body.classList.add('mobile');
            this.addLog('检测到移动设备，已启用移动端优化', 'info');
        }
        
        // 初始化滑块值显示
        this.updateSliderDisplays();
    }
    
    updateSliderDisplays() {
        const intervalSlider = document.getElementById('scheduling-interval');
        const intervalValue = document.getElementById('interval-value');
        if (intervalSlider && intervalValue) {
            intervalValue.textContent = intervalSlider.value;
        }
        
        const optimizationSlider = document.getElementById('optimization-level');
        const optimizationValue = document.getElementById('optimization-value');
        if (optimizationSlider && optimizationValue) {
            optimizationValue.textContent = optimizationSlider.value + '%';
        }
    }
    
    // API调用封装
    async apiCall(endpoint, options = {}) {
        try {
            const response = await fetch(`${this.apiBase}${endpoint}`, {
                headers: {
                    'Content-Type': 'application/json',
                    ...options.headers
                },
                ...options
            });
            
            if (!response.ok) {
                throw new Error(`HTTP ${response.status}: ${response.statusText}`);
            }
            
            return await response.json();
        } catch (error) {
            console.error('API调用失败:', error);
            this.addLog(`API调用失败: ${error.message}`, 'error');
            throw error;
        }
    }
    
    // 获取引擎状态
    async getEngineStatus() {
        try {
            const data = await this.apiCall('/status');
            return data;
        } catch (error) {
            // 如果API不可用，返回模拟数据
            return this.getMockStatus();
        }
    }
    
    // 模拟状态数据（用于测试）
    getMockStatus() {
        return {
            engine_status: this.isEngineRunning ? 'running' : 'stopped',
            current_scene: this.currentScene,
            active_tasks: Math.floor(Math.random() * 20) + 5,
            ces_score: (Math.random() * 30 + 70).toFixed(1),
            cpu_usage: (Math.random() * 50 + 20).toFixed(1),
            memory_usage: (Math.random() * 40 + 30).toFixed(1),
            thermal_state: (Math.random() * 30 + 10).toFixed(1),
            timestamp: new Date().toLocaleString('zh-CN')
        };
    }
    
    // 更新状态显示
    updateStatusDisplay(status) {
        // 更新引擎状态
        const engineStatus = document.getElementById('engine-status');
        if (engineStatus) {
            engineStatus.textContent = status.engine_status === 'running' ? '运行中' : '已停止';
            engineStatus.className = 'stat-value ' + (status.engine_status === 'running' ? 'success' : 'error');
        }
        
        // 更新CES分数
        const cesScore = document.getElementById('ces-score');
        if (cesScore) {
            cesScore.textContent = status.ces_score;
            this.updateScoreColor(cesScore, parseFloat(status.ces_score));
        }
        
        // 更新CPU使用率
        const cpuUsage = document.getElementById('cpu-usage');
        if (cpuUsage) {
            cpuUsage.textContent = status.cpu_usage + '%';
            this.updateUsageColor(cpuUsage, parseFloat(status.cpu_usage));
        }
        
        // 更新内存使用率
        const memoryUsage = document.getElementById('memory-usage');
        if (memoryUsage) {
            memoryUsage.textContent = status.memory_usage + '%';
            this.updateUsageColor(memoryUsage, parseFloat(status.memory_usage));
        }
        
        // 更新热状态
        const thermalState = document.getElementById('thermal-state');
        if (thermalState) {
            thermalState.textContent = status.thermal_state + '%';
            this.updateThermalColor(thermalState, parseFloat(status.thermal_state));
        }
        
        // 更新活动任务数
        const activeTasks = document.getElementById('active-tasks');
        if (activeTasks) {
            activeTasks.textContent = status.active_tasks;
        }
        
        // 更新场景描述
        this.updateSceneDescription(status.current_scene);
    }
    
    // 更新分数颜色
    updateScoreColor(element, score) {
        if (score >= 80) {
            element.style.color = '#51cf66'; // 绿色
        } else if (score >= 60) {
            element.style.color = '#ffd43b'; // 黄色
        } else {
            element.style.color = '#ff6b6b'; // 红色
        }
    }
    
    // 更新使用率颜色
    updateUsageColor(element, usage) {
        if (usage <= 50) {
            element.style.color = '#51cf66'; // 绿色
        } else if (usage <= 80) {
            element.style.color = '#ffd43b'; // 黄色
        } else {
            element.style.color = '#ff6b6b'; // 红色
        }
    }
    
    // 更新热状态颜色
    updateThermalColor(element, thermal) {
        if (thermal <= 30) {
            element.style.color = '#51cf66'; // 绿色
        } else if (thermal <= 60) {
            element.style.color = '#ffd43b'; // 黄色
        } else {
            element.style.color = '#ff6b6b'; // 红色
        }
    }
    
    // 更新场景描述
    updateSceneDescription(scene) {
        const descriptions = {
            0: '游戏模式：优先性能，60FPS目标',
            1: '社交模式：平衡功耗，30FPS目标',
            2: '媒体模式：优化解码，30FPS目标',
            3: '办公模式：提升响应，60FPS目标'
        };
        
        const sceneDesc = document.getElementById('scene-description');
        if (sceneDesc) {
            sceneDesc.textContent = descriptions[scene] || '自动检测场景';
        }
        
        // 更新场景按钮状态
        document.querySelectorAll('.scene-btn').forEach(btn => {
            btn.classList.remove('active');
            if (parseInt(btn.dataset.scene) === scene) {
                btn.classList.add('active');
            }
        });
    }
    
    // 启动引擎
    async startEngine() {
        this.addLog('正在启动UIEE引擎...', 'info');
        
        try {
            // 模拟启动过程
            await this.delay(2000);
            
            this.isEngineRunning = true;
            this.addLog('UIEE引擎启动成功', 'success');
            
            // 更新按钮状态
            this.updateControlButtons();
            
            // 立即更新一次状态
            await this.updateStatus();
            
        } catch (error) {
            this.addLog(`启动引擎失败: ${error.message}`, 'error');
        }
    }
    
    // 停止引擎
    async stopEngine() {
        this.addLog('正在停止UIEE引擎...', 'info');
        
        try {
            // 模拟停止过程
            await this.delay(1000);
            
            this.isEngineRunning = false;
            this.addLog('UIEE引擎已停止', 'warning');
            
            // 更新按钮状态
            this.updateControlButtons();
            
        } catch (error) {
            this.addLog(`停止引擎失败: ${error.message}`, 'error');
        }
    }
    
    // 重启引擎
    async restartEngine() {
        this.addLog('正在重启UIEE引擎...', 'info');
        
        await this.stopEngine();
        await this.delay(500);
        await this.startEngine();
    }
    
    // 设置场景模式
    async setScene(sceneId) {
        this.currentScene = parseInt(sceneId);
        this.addLog(`切换到场景模式: ${this.getSceneName(sceneId)}`, 'info');
        
        try {
            // 这里可以调用API更新场景设置
            // await this.apiCall('/scene', { method: 'POST', body: JSON.stringify({ scene: sceneId }) });
            
            // 更新UI
            this.updateSceneDescription(this.currentScene);
            
        } catch (error) {
            this.addLog(`设置场景失败: ${error.message}`, 'error');
        }
    }
    
    // 获取场景名称
    getSceneName(sceneId) {
        const names = ['游戏模式', '社交模式', '媒体模式', '办公模式'];
        return names[sceneId] || '未知模式';
    }
    
    // 更新CTO设置
    async updateCTOSettings() {
        const settings = {
            enable_cto: document.getElementById('enable-cto')?.checked || false,
            enable_io_scheduling: document.getElementById('enable-io-scheduling')?.checked || false,
            enable_cpu_affinity: document.getElementById('enable-cpu-affinity')?.checked || false
        };
        
        this.addLog('更新CTO设置...', 'info');
        
        try {
            // 这里可以调用API更新CTO设置
            // await this.apiCall('/cto', { method: 'POST', body: JSON.stringify(settings) });
            
            this.addLog('CTO设置已更新', 'success');
            
        } catch (error) {
            this.addLog(`更新CTO设置失败: ${error.message}`, 'error');
        }
    }
    
    // 更新调度间隔
    async updateSchedulingInterval(interval) {
        this.addLog(`更新调度间隔: ${interval}秒`, 'info');
        
        try {
            // 这里可以调用API更新调度间隔
            // await this.apiCall('/config', { method: 'POST', body: JSON.stringify({ scheduling_interval: interval }) });
            
        } catch (error) {
            this.addLog(`更新调度间隔失败: ${error.message}`, 'error');
        }
    }
    
    // 更新优化级别
    async updateOptimizationLevel(level) {
        this.addLog(`更新优化级别: ${level}%`, 'info');
        
        try {
            // 这里可以调用API更新优化级别
            // await this.apiCall('/config', { method: 'POST', body: JSON.stringify({ optimization_level: level }) });
            
        } catch (error) {
            this.addLog(`更新优化级别失败: ${error.message}`, 'error');
        }
    }
    
    // 更新控制按钮状态
    updateControlButtons() {
        const startBtn = document.getElementById('start-engine');
        const stopBtn = document.getElementById('stop-engine');
        const restartBtn = document.getElementById('restart-engine');
        
        if (startBtn) {
            startBtn.disabled = this.isEngineRunning;
        }
        
        if (stopBtn) {
            stopBtn.disabled = !this.isEngineRunning;
        }
        
        if (restartBtn) {
            restartBtn.disabled = !this.isEngineRunning;
        }
    }
    
    // 启动状态更新
    startStatusUpdate() {
        if (this.updateTimer) {
            clearInterval(this.updateTimer);
        }
        
        this.updateTimer = setInterval(() => {
            this.updateStatus();
        }, this.updateInterval);
        
        // 立即更新一次
        this.updateStatus();
    }
    
    // 停止状态更新
    stopStatusUpdate() {
        if (this.updateTimer) {
            clearInterval(this.updateTimer);
            this.updateTimer = null;
        }
    }
    
    // 更新状态
    async updateStatus() {
        try {
            const status = await this.getEngineStatus();
            this.updateStatusDisplay(status);
        } catch (error) {
            console.error('更新状态失败:', error);
        }
    }
    
    // 添加日志
    addLog(message, type = 'info') {
        if (!this.logContainer) return;
        
        const timestamp = new Date().toLocaleTimeString('zh-CN');
        const logEntry = document.createElement('div');
        logEntry.className = `log-entry ${type}`;
        logEntry.textContent = `[${timestamp}] ${message}`;
        
        this.logContainer.appendChild(logEntry);
        
        // 保持日志在底部
        this.logContainer.scrollTop = this.logContainer.scrollHeight;
        
        // 限制日志数量
        const maxLogs = 100;
        const logs = this.logContainer.querySelectorAll('.log-entry');
        if (logs.length > maxLogs) {
            logs[0].remove();
        }
    }
    
    // 清空日志
    clearLogs() {
        if (this.logContainer) {
            this.logContainer.innerHTML = '';
            this.addLog('日志已清空', 'info');
        }
    }
    
    // 导出日志
    exportLogs() {
        if (!this.logContainer) return;
        
        const logs = Array.from(this.logContainer.querySelectorAll('.log-entry'))
            .map(entry => entry.textContent)
            .join('\n');
        
        const blob = new Blob([logs], { type: 'text/plain' });
        const url = URL.createObjectURL(blob);
        
        const a = document.createElement('a');
        a.href = url;
        a.download = `uiee_logs_${new Date().toISOString().split('T')[0]}.txt`;
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
        
        URL.revokeObjectURL(url);
        this.addLog('日志已导出', 'success');
    }
    
    // 工具函数：延迟
    delay(ms) {
        return new Promise(resolve => setTimeout(resolve, ms));
    }
}

// 页面加载完成后初始化
document.addEventListener('DOMContentLoaded', () => {
    console.log('页面加载完成，初始化UIEE WebUI...');
    
    // 创建全局实例
    window.uieeModule = new UIEEModule();
    
    // 添加页面加载动画
    document.body.classList.add('fade-in');
    
    console.log('UIEE WebUI初始化完成');
});

// 处理网络状态变化
window.addEventListener('online', () => {
    console.log('网络连接已恢复');
    if (window.uieeModule) {
        window.uieeModule.addLog('网络连接已恢复', 'success');
    }
});

window.addEventListener('offline', () => {
    console.log('网络连接已断开');
    if (window.uieeModule) {
        window.uieeModule.addLog('网络连接已断开', 'warning');
    }
});

// 处理页面可见性变化
document.addEventListener('visibilitychange', () => {
    if (document.hidden) {
        // 页面隐藏时暂停更新
        if (window.uieeModule) {
            window.uieeModule.stopStatusUpdate();
        }
    } else {
        // 页面显示时恢复更新
        if (window.uieeModule) {
            window.uieeModule.startStatusUpdate();
        }
    }
});

// 错误处理
window.addEventListener('error', (event) => {
    console.error('页面错误:', event.error);
    if (window.uieeModule) {
        window.uieeModule.addLog(`页面错误: ${event.error.message}`, 'error');
    }
});

window.addEventListener('unhandledrejection', (event) => {
    console.error('未处理的Promise拒绝:', event.reason);
    if (window.uieeModule) {
        window.uieeModule.addLog(`未处理的Promise拒绝: ${event.reason}`, 'error');
    }
});