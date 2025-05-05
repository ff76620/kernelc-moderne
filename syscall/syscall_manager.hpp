#ifndef SYSCALL_MANAGER_HPP
#define SYSCALL_MANAGER_HPP

#include 
#include 
#include "../include/types.hpp"

namespace Kernel {
namespace Syscall {

class SyscallManager {
public:
    SyscallManager();
    ~SyscallManager();

    void initialize();
    void shutdown();

    // Syscall Registration
    void registerSyscall(SyscallID id, std::function handler);
    void unregisterSyscall(SyscallID id);
    
    // Syscall Processing
    long handleSyscall(SyscallContext* context);
    void validateSyscall(SyscallContext* context);
    
    // Syscall Management
    struct SyscallStats {
        uint64_t totalCalls;
        uint64_t failedCalls;
        std::unordered_map callCounts;
        double avgProcessingTime;
    };

    SyscallStats getStats() const;
    void clearStats();

private:
    std::unordered_map handlers;
    SyscallConfiguration config;
    
    void setupSyscallTable();
    void validateParameters(const SyscallContext* context);
    void logSyscall(const SyscallContext* context);
};

} // namespace Syscall
} // namespace Kernel

#endif