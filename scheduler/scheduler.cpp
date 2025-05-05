#include "kernel/scheduler/scheduler.hpp"
#include "kernel/loggin/EventLogger.hpp"
#include "kernel/process/process.hpp"
#include "kernel/interrupt/InterruptManager.hpp"

namespace Kernel {

class EnhancedScheduler {
private:
    static constexpr size_t MAX_PROCESSES = 1024;
    static constexpr size_t MAX_THREADS = 256;
    static constexpr size_t TIME_SLICE = 1; // ms
    
    struct ThreadContext {
        uint64_t priority;
        uint64_t cpuTime;
        bool isGameThread;
        std::atomic isActive;
        ProcessContext* processContext;
    };

    std::array threads;
    std::atomic activeThreadCount{0};
    std::vector> processes;
    std::queue readyQueue;
    Process* currentProcess;
    ThreadContext* currentThread;
    State state;

public:
    EnhancedScheduler() : currentProcess(nullptr), state(State::STOPPED) {}
    
    ~EnhancedScheduler() {
        stop();
    }

    static EnhancedScheduler& getInstance() {
        static EnhancedScheduler instance;
        return instance;
    }

    void initialize() {
        EventLogger::log("Initializing enhanced scheduler...");
        processes.reserve(MAX_PROCESSES);
        setupTimerInterrupt();
        readyQueue = std::queue();
        setupPriorities();
        state = State::STOPPED;
        EventLogger::log("Enhanced scheduler initialized successfully");
    }

    void start() {
        if(state == State::RUNNING) return;
        state = State::RUNNING;
        InterruptManager::getInstance().enableInterrupt(0);
        EventLogger::log("Enhanced scheduler started");
    }

    void stop() {
        if(state != State::RUNNING) return;
        state = State::STOPPED;
        InterruptManager::getInstance().disableInterrupt(0);
        EventLogger::log("Enhanced scheduler stopped");
    }

    ProcessID createProcess(void* entryPoint, uint32_t priority) {
        if(processes.size() >= MAX_PROCESSES) {
            EventLogger::log("Error: Maximum process limit reached");
            return 0;
        }
        
        ProcessID pid = processes.size() + 1;
        auto process = std::make_unique(pid, entryPoint, priority);
        process->setState(Process::State::READY);
        
        // Create main thread for process
        ThreadContext& thread = threads[activeThreadCount++];
        thread.priority = priority;
        thread.cpuTime = 0;
        thread.isGameThread = (priority > 10); // Assume high priority = game thread
        thread.isActive = true;
        thread.processContext = process->getContext();
        
        readyQueue.push(process.get());
        processes.push_back(std::move(process));
        
        return pid;
    }

    void terminateProcess(ProcessID pid) {
        for(auto& process : processes) {
            if(process->getID() == pid) {
                process->setState(Process::State::TERMINATED);
                // Clean up associated threads
                for(size_t i = 0; i < activeThreadCount; i++) {
                    if(threads[i].processContext == process->getContext()) {
                        threads[i].isActive = false;
                    }
                }
                break;
            }
        }
    }

    void schedule() {
        if(state != State::RUNNING) return;

        // Game-aware scheduling
        ThreadContext* nextThread = nullptr;
        uint64_t maxPriority = 0;

        #pragma omp parallel for reduction(max:maxPriority)
        for(size_t i = 0; i < activeThreadCount; i++) {
            if(threads[i].isActive) {
                uint64_t dynamicPriority = calculateDynamicPriority(threads[i]);
                if(dynamicPriority > maxPriority) {
                    maxPriority = dynamicPriority;
                    nextThread = &threads[i];
                }
            }
        }

        if(nextThread && nextThread != currentThread) {
            Process* nextProcess = nullptr;
            // Find process associated with thread
            for(auto& process : processes) {
                if(process->getContext() == nextThread->processContext) {
                    nextProcess = process.get();
                    break;
                }
            }

            if(nextProcess && nextProcess != currentProcess) {
                if(currentProcess) {
                    saveProcess(currentProcess);
                    currentProcess->setState(Process::State::READY);
                    readyQueue.push(currentProcess);
                }
                
                loadProcess(nextProcess);
                currentProcess = nextProcess;
                currentThread = nextThread;
                currentProcess->setState(Process::State::RUNNING);
            }
        }
    }

private:
    uint64_t calculateDynamicPriority(const ThreadContext& thread) {
        uint64_t priority = thread.priority;
        if(thread.isGameThread) {
            priority *= 2;  // Boost game threads
        }
        // Add CPU fairness factor
        priority += (1000000 / (thread.cpuTime + 1));
        return priority;
    }

    void setupTimerInterrupt() {
        uint32_t divisor = 1193180 / (1000 / TIME_SLICE);
        outb(0x43, 0x36);
        outb(0x40, divisor & 0xFF);
        outb(0x40, (divisor >> 8) & 0xFF);
        InterruptManager::getInstance().registerHandler(0, timerHandler);
    }

    void loadProcess(Process* process) {
        if(!process) return;
        
        ProcessContext* context = process->getContext();
        asm volatile(
            "movq %0, %%rsp\n"
            "popq %%rax\n"
            "popq %%rbx\n"
            "popq %%rcx\n"
            "popq %%rdx\n"
            "popq %%rsi\n"
            "popq %%rdi\n"
            "popq %%rbp\n"
            "popq %%r8\n"
            "popq %%r9\n"
            "popq %%r10\n"
            "popq %%r11\n"
            "popq %%r12\n"
            "popq %%r13\n"
            "popq %%r14\n"
            "popq %%r15\n"
            "iretq"
            :
            : "g"(context)
            : "memory"
        );
    }

    void saveProcess(Process* process) {
        if(!process) return;
        
        ProcessContext* context = process->getContext();
        asm volatile(
            "pushq %%rax\n"
            "pushq %%rbx\n"
            "pushq %%rcx\n"
            "pushq %%rdx\n"
            "pushq %%rsi\n"
            "pushq %%rdi\n"
            "pushq %%rbp\n"
            "pushq %%r8\n"
            "pushq %%r9\n"
            "pushq %%r10\n"
            "pushq %%r11\n"
            "pushq %%r12\n"
            "pushq %%r13\n"
            "pushq %%r14\n"
            "pushq %%r15\n"
            "movq %%rsp, %0"
            : "=m"(context)
            :
            : "memory"
        );
    }

    static void timerHandler(void* context) {
        getInstance().schedule();
    }
};

} // namespace Kernel
        