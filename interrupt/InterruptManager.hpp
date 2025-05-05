#ifndef INTERRUPT_MANAGER_HPP
#define INTERRUPT_MANAGER_HPP

#include "../include/types.hpp"
#include "../interrupt/InterruptHandler.hpp"
#include 
#include 
#include 

namespace Kernel {
namespace Interrupt {

class InterruptManager {
public:
    static InterruptManager& getInstance();

    bool initialize();
    void shutdown();

    // Register handler for specific interrupt
    bool registerHandler(uint32_t interruptNumber, std::shared_ptr handler);
    bool unregisterHandler(uint32_t interruptNumber);

    // Enable/disable interrupts
    void enableInterrupts();
    void disableInterrupts();

    // Handle interrupt
    void handleInterrupt(uint32_t interruptNumber);

    // IRQ management
    void maskIRQ(uint32_t irqNumber);
    void unmaskIRQ(uint32_t irqNumber);

private:
    InterruptManager() = default;
    ~InterruptManager() = default;

    struct InterruptDescriptor {
        std::shared_ptr handler;
        bool enabled;
    };

    std::vector interruptHandlers;
    bool interruptsEnabled;

    void setupIDT();
    void setupPIC();
    
    // Prevent copying
    InterruptManager(const InterruptManager&) = delete;
    InterruptManager& operator=(const InterruptManager&) = delete;
};

} // namespace Interrupt
} // namespace Kernel

#endif