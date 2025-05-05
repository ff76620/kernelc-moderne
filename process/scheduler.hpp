#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include "../include/types.hpp"
#include "../process/process.hpp"
#include "../interrupt/InterruptManager.hpp"
#include 
#include 
#include 

namespace Kernel {

class Scheduler {
public:
    static Scheduler& getInstance();
    
    void initialize();
    void start();
    void stop();
    
    ProcessID createProcess(void* entryPoint, uint32_t priority = 1);
    void terminateProcess(ProcessID pid);
    void yield();
    
    Process* getCurrentProcess();
    void schedule();
    
    enum class State {
        STOPPED,
        RUNNING,
        ERROR
    };
    
    State getState() const { return state; }
    
private:
    Scheduler();
    ~Scheduler();
    
    static const uint32_t MAX_PROCESSES = 1024;
    static const uint32_t TIME_SLICE = 10; // ms
    
    std::vector> processes;
    std::priority_queue readyQueue;
    Process* currentProcess;
    State state;
    
    void setupTimerInterrupt();
    void loadProcess(Process* process);
    void saveProcess(Process* process);
    void switchContext();
    
    static void timerHandler(void* context);
};

} // namespace Kernel

#endif