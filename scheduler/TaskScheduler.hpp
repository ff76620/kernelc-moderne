#ifndef TASK_SCHEDULER_HPP
#define TASK_SCHEDULER_HPP

#include 
#include 
#include 
#include 

namespace Kernel {

class TaskScheduler {
private:
    struct ProcessPriorityManager {
        void setGameProcessPriority(int priority) { /* Implementation */ }
        void setBackgroundProcessPriority(int priority) { /* Implementation */ }
        void initializeGamingProfile() { /* Implementation */ }
    };

    struct ThreadPool {
        void setThreadCount(int count) { /* Implementation */ }
        void setThreadPriority(int priority) { /* Implementation */ }
        void optimizeForGaming() { /* Implementation */ }
    };

    struct IOScheduler {
        void prioritizeGameIO() { /* Implementation */ }
        void minimizeLatency() { /* Implementation */ }
        void setupGameMode() { /* Implementation */ }
    };

    ProcessPriorityManager m_priorityManager;
    ThreadPool m_renderThreadPool;
    IOScheduler m_ioScheduler;
    GameMetrics metrics;
    
    std::priority_queue> readyQueue;
    std::map> processes;
    std::shared_ptr currentProcess;
    
    uint32_t timeSlice;
    bool running;
    LoadBalancer loadBalancer;

    static TaskScheduler* instance;

public:
    static TaskScheduler& getInstance() {
        if (!instance) {
            instance = new TaskScheduler();
        }
        return *instance;
    }

    void initialize() {
        timeSlice = 10; // 10ms default time slice
        running = false;
        setupGamingOptimizations();
    }

    void optimizeForGaming() {
        // Configure CPU priorities
        m_priorityManager.setGameProcessPriority(HIGH);
        m_priorityManager.setBackgroundProcessPriority(LOW);
        
        // Optimize render threads
        m_renderThreadPool.setThreadCount(8);
        m_renderThreadPool.setThreadPriority(HIGH);
        
        // IO optimization
        m_ioScheduler.prioritizeGameIO();
        m_ioScheduler.minimizeLatency();
        
        // Game-aware adaptations
        adaptToGameLoad();
    }
    
    void adaptToGameLoad() {
        metrics.analyze();
        adjustPriorities();
        loadBalancer.prioritizeGameTasks();
    }
    
    void enableGameMode() {
        suspendNonEssentialProcesses();
        optimizeForGaming();
        enableLatencyOptimization();
    }

    void scheduleTask(Task task) {
        readyQueue.push(std::make_shared(task));
        balanceLoad();
    }

    void start() {
        running = true;
        schedule();
    }

    void stop() {
        running = false;
    }

    void addProcess(std::shared_ptr process) {
        processes[process->getPid()] = process;
        readyQueue.push(process);
    }

    void removeProcess(pid_t pid) {
        processes.erase(pid);
        rebuildReadyQueue();
    }
    
    void yield() {
        if (currentProcess) {
            readyQueue.push(currentProcess);
        }
        schedule();
    }

    void switchContext() {
        if (!readyQueue.empty()) {
            auto next = readyQueue.top();
            readyQueue.pop();
            contextSwitch(next);
        }
    }
    
    std::shared_ptr getCurrentProcess() const {
        return currentProcess;
    }

    size_t getProcessCount() const {
        return processes.size();
    }

    void setTimeSlice(uint32_t milliseconds) {
        timeSlice = milliseconds;
    }

    uint32_t getTimeSlice() const {
        return timeSlice;
    }

private:
    TaskScheduler() = default;
    ~TaskScheduler() = default;

    void schedule() {
        while (running) {
            if (!readyQueue.empty()) {
                switchContext();
            }
        }
    }

    void contextSwitch(std::shared_ptr next) {
        if (currentProcess) {
            currentProcess->save();
        }
        currentProcess = next;
        currentProcess->restore();
    }

    void rebuildReadyQueue() {
        std::priority_queue> newQueue;
        for (const auto& pair : processes) {
            newQueue.push(pair.second);
        }
        readyQueue = std::move(newQueue);
    }

    void setupGamingOptimizations() {
        m_priorityManager.initializeGamingProfile();
        m_renderThreadPool.optimizeForGaming();
        m_ioScheduler.setupGameMode();
    }
    
    void balanceLoad() {
        loadBalancer.distribute(readyQueue);
    }
    
    void adjustPriorities() {
        for (auto& process : processes) {
            process.second->updatePriority(metrics.getLoadFactor());
        }
    }
};

} // namespace Kernel

#endif