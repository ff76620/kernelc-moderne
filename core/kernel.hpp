#ifndef KERNEL_HPP
#define KERNEL_HPP

#include "../include/types.hpp"
#include "../scheduler/scheduler.hpp"
#include "../memory/vmm.hpp"
#include "../interrupt/InterruptManager.hpp"
#include "../drivers/DriverManager.hpp"

namespace Kernel {

class KernelSystem {
public:
    static void initialize();
    static void start();
    
    // Core system managers
    static InterruptManager* interruptManager;
    static SchedulerSystem* scheduler;
    static VirtualMemoryManager* memoryManager;
    static DriverManager* driverManager;

private:
    static void initializeSubsystems();
    static void setupHardware();
    static void initializeDrivers();
    static void startSystemServices();
    
    static bool initialized;
};

} // namespace Kernel

#endif // KERNEL_HPP