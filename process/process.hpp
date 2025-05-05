#ifndef PROCESS_HPP
#define PROCESS_HPP

#include "../include/types.hpp"
#include 

namespace Kernel {

using ProcessID = uint32_t;

struct ProcessContext {
    uint64_t rax, rbx, rcx, rdx;
    uint64_t rsi, rdi, rsp, rbp;
    uint64_t r8, r9, r10, r11, r12, r13, r14, r15;
    uint64_t rip, rflags;
    uint64_t cs, ss, ds, es, fs, gs;
};

class Process {
public:
    enum class State {
        CREATED,
        READY,
        RUNNING,
        BLOCKED,
        TERMINATED
    };
    
    Process(ProcessID id, void* entryPoint, uint32_t priority);
    ~Process();
    
    ProcessID getID() const { return id; }
    State getState() const { return state; }
    uint32_t getPriority() const { return priority; }
    
    void setState(State newState) { state = newState; }
    void setPriority(uint32_t newPriority) { priority = newPriority; }
    
    ProcessContext* getContext() { return &context; }
    
    bool operator<(const Process& other) const {
        return priority < other.priority;
    }
    
private:
    ProcessID id;
    State state;
    uint32_t priority;
    ProcessContext context;
    void* stackBase;
    size_t stackSize;
    
    static const size_t DEFAULT_STACK_SIZE = 64 * 1024; // 64KB
    
    void initializeStack(void* entryPoint);
};

} // namespace Kernel

#endif