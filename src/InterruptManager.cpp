#include "../interrupt/InterruptManager.hpp"
#include "../loggin/EventLogger.hpp"
#include 

namespace Kernel {
namespace Interrupt {

InterruptManager& InterruptManager::getInstance() {
    static InterruptManager instance;
    return instance;
}

bool InterruptManager::initialize() {
    EventLogger::log("Initializing Interrupt Manager...");
    
    try {
        interruptHandlers.resize(256); // Support for 256 interrupts
        interruptsEnabled = false;

        setupIDT();
        setupPIC();
        
        EventLogger::log("Interrupt Manager initialized successfully");
        return true;
    }
    catch (const std::exception& e) {
        EventLogger::log("Error initializing Interrupt Manager: " + std::string(e.what()));
        return false;
    }
}

void InterruptManager::shutdown() {
    EventLogger::log("Shutting down Interrupt Manager...");
    disableInterrupts();
    interruptHandlers.clear();
    EventLogger::log("Interrupt Manager shutdown complete");
}

bool InterruptManager::registerHandler(uint32_t interruptNumber, std::shared_ptr handler) {
    if (interruptNumber >= interruptHandlers.size()) {
        return false;
    }

    interruptHandlers[interruptNumber].handler = handler;
    interruptHandlers[interruptNumber].enabled = true;
    return true;
}

bool InterruptManager::unregisterHandler(uint32_t interruptNumber) {
    if (interruptNumber >= interruptHandlers.size()) {
        return false;
    }

    interruptHandlers[interruptNumber].handler = nullptr;
    interruptHandlers[interruptNumber].enabled = false;
    return true;
}

void InterruptManager::enableInterrupts() {
    _mm_lfence(); // Memory fence before enabling interrupts
    asm volatile("sti");
    interruptsEnabled = true;
}

void InterruptManager::disableInterrupts() {
    asm volatile("cli");
    _mm_mfence(); // Memory fence after disabling interrupts
    interruptsEnabled = false;
}

void InterruptManager::handleInterrupt(uint32_t interruptNumber) {
    if (interruptNumber < interruptHandlers.size() && 
        interruptHandlers[interruptNumber].enabled &&
        interruptHandlers[interruptNumber].handler) {
        
        interruptHandlers[interruptNumber].handler->handle(interruptNumber);
    }
}

void InterruptManager::maskIRQ(uint32_t irqNumber) {
    if (irqNumber < 16) {
        uint16_t port = (irqNumber < 8) ? 0x21 : 0xA1;
        uint8_t value = inb(port) | (1 << (irqNumber % 8));
        outb(port, value);
    }
}

void InterruptManager::unmaskIRQ(uint32_t irqNumber) {
    if (irqNumber < 16) {
        uint16_t port = (irqNumber < 8) ? 0x21 : 0xA1;
        uint8_t value = inb(port) & ~(1 << (irqNumber % 8));
        outb(port, value);
    }
}

void InterruptManager::setupIDT() {
    // Implementation of IDT setup
    // This would involve creating and loading the IDT
    // and setting up exception handlers
}

void InterruptManager::setupPIC() {
    // Initialize the 8259 PIC
    outb(0x20, 0x11); // Initialize master PIC
    outb(0xA0, 0x11); // Initialize slave PIC
    
    // Set up IRQ mappings
    outb(0x21, 0x20); // Master PIC vector offset
    outb(0xA1, 0x28); // Slave PIC vector offset
    
    // Set up master/slave relationship
    outb(0x21, 0x04); // Tell master about slave
    outb(0xA1, 0x02); // Tell slave its cascade identity
    
    // Set 8086/88 mode
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    
    // Mask all interrupts initially
    outb(0x21, 0xFF);
    outb(0xA1, 0xFF);
}

} // namespace Interrupt
} // namespace Kernel