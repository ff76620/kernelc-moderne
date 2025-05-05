#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include 
#include 
#include 
#include 
#include 

namespace Kernel {

class Thread;
class Process;

enum class ProcessState {
    READY,
    RUNNING,
    BLOCKED,
    TERMINATED
};

enum class SchedulerPolicy {
    ROUND_ROBIN,
    PRIORITY,
    REALTIME
};

struct ProcessPriority {
    int base;
    int dynamic;
    
    ProcessPriority(int b = 0) : base(b), dynamic(b) {}
};

class Scheduler {
public:
    Scheduler(SchedulerPolicy policy = SchedulerPolicy::ROUND_ROBIN);
    ~Scheduler();

    // Process management
    int createProcess(std::function entry_point, ProcessPriority priority = ProcessPriority());
    bool terminateProcess(int pid);
    bool blockProcess(int pid);
    bool unblockProcess(int pid);

    // Thread management
    int createThread(int pid, std::function entry_point);
    bool terminateThread(int tid);

    // Scheduling operations
    void schedule();
    void yield();
    void tick();

    // Configuration
    void setTimeSlice(unsigned int ms);
    void setSchedulingPolicy(SchedulerPolicy policy);
    
    // Statistics
    double getCPUUsage() const;
    int getActiveProcessCount() const;
    int getActiveThreadCount() const;

private:
    struct ProcessContext {
        std::shared_ptr process;
        ProcessState state;
        ProcessPriority priority;
        std::vector> threads;
        unsigned long runtime;
        
        ProcessContext(std::shared_ptr p, ProcessPriority prio)
            : process(p), state(ProcessState::READY), priority(prio), runtime(0) {}
    };

    SchedulerPolicy currentPolicy;
    unsigned int timeSlice;
    std::atomic nextPID;
    std::atomic nextTID;
    
    std::shared_ptr currentProcess;
    std::shared_ptr currentThread;
    
    std::vector> processes;
    std::queue> readyQueue;

    void contextSwitch(std::shared_ptr next_process, std::shared_ptr next_thread);
    void updatePriorities();
    std::shared_ptr selectNextProcess();
    void preemptCurrentProcess();
};

} // namespace Kernel

#endif // SCHEDULER_HPP