#ifndef INTERRUPT_HANDLER_HPP
#define INTERRUPT_HANDLER_HPP

#include 
#include 
#include 
#include 
#include 
#include "../include/types.hpp"
#include "../arch/x86_64/cpu.hpp"

namespace Kernel {

using ISRFunction = std::function<void(void*) w-tid="15">;

class InterruptServiceRoutine {
private:
    ISRFunction handler;
    uint32_t number;
    const char* name;
    
public:
    void execute(void* context) {
        if (handler) handler(context);
    }
};

class InterruptHandler {
public:
    enum class Priority {
        LOW = 0,
        NORMAL = 1,
        HIGH = 2,
        CRITICAL = 3
    };

private:
    std::array isrTable;
    std::vector priorityLevels;
    bool interruptsEnabled;

public:
    virtual ~InterruptHandler() = default;
    
    void handleInterrupt(uint8_t vector, void* context) {
        if (interruptsEnabled) {
            isrTable[vector].execute(context);
        }
    }
    
    void registerISR(uint8_t vector, ISRFunction handler, 
                    const char* name, Priority priority = Priority::NORMAL) {
        isrTable[vector].handler = handler;
        isrTable[vector].number = vector;
        isrTable[vector].name = name;
        priorityLevels[vector] = static_cast(priority);
    }

    virtual uint32_t getInterruptNumber() const { 
        return currentInterrupt; 
    }
    
    virtual const char* getName() const { 
        return isrTable[currentInterrupt].name; 
    }
    
    virtual Priority getPriority() const { 
        return static_cast(priorityLevels[currentInterrupt]); 
    }

    void enableInterrupts() {
        _mm_lfence(); // Memory barrier
        __asm__ volatile("sti");
        interruptsEnabled = true;
    }

    void disableInterrupts() {
        __asm__ volatile("cli");
        _mm_sfence(); // Memory barrier
        interruptsEnabled = false;
    }

private:
    uint32_t currentInterrupt = 0;
};

class InterruptManager {
private:
    std::unique_ptr handler;
    std::vector idt;
    bool initialized = false;
    static constexpr uint16_t IDT_PRESENT = 0x8000;
    static constexpr uint16_t IDT_DPL0 = 0x0000;
    static constexpr uint16_t IDT_DPL3 = 0x6000;
    static constexpr uint16_t IDT_INTERRUPT = 0x0E00;
    static constexpr uint16_t IDT_TRAP = 0x0F00;

public:
    InterruptManager() : handler(std::make_unique()) {
        idt.resize(256);
    }

    void initialize() {
        if (!initialized) {
            setupIDT();
            handler->enableInterrupts();
            initialized = true;
        }
    }

    void registerInterruptHandler(uint8_t vector, ISRFunction handler,
                                const char* name, 
                                InterruptHandler::Priority priority = InterruptHandler::Priority::NORMAL) {
        this->handler->registerISR(vector, handler, name, priority);
        updateIDTEntry(vector);
    }

private:
    void setupIDT() {
        for (int i = 0; i < 256; i++) {
            setupIDTEntry(i, defaultHandler);
        }
        loadIDT();
    }

    void setupIDTEntry(uint8_t vector, void (*handler)()) {
        uint64_t entry = createIDTEntry(handler);
        idt[vector] = entry;
    }

    void updateIDTEntry(uint8_t vector) {
        uint64_t entry = createIDTEntry(
            [this, vector](void* context) {
                handler->handleInterrupt(vector, context);
            }
        );
        idt[vector] = entry;
    }

    static void defaultHandler() {
        __asm__ volatile(
            "push %rax\n\t"
            "push %rcx\n\t"
            "push %rdx\n\t"
            "push %rbx\n\t"
            "push %rbp\n\t"
            "push %rsi\n\t"
            "push %rdi\n\t"
        );
        
        // Handle spurious interrupt
        uint32_t interruptNumber;
        __asm__ volatile("mov %%rax, %0" : "=r"(interruptNumber));
        
        // Send EOI if necessary
        if (interruptNumber >= 32) {
            outportb(0x20, 0x20);
            if (interruptNumber >= 40) {
                outportb(0xA0, 0x20);
            }
        }
        
        __asm__ volatile(
            "pop %rdi\n\t"
            "pop %rsi\n\t" 
            "pop %rbp\n\t"
            "pop %rbx\n\t"
            "pop %rdx\n\t"
            "pop %rcx\n\t"
            "pop %rax\n\t"
            "iretq"
        );
    }

    uint64_t createIDTEntry(void (*handler)()) {
        uint64_t handlerAddr = reinterpret_cast(handler);
        
        uint64_t entry = 0;
        entry |= (handlerAddr & 0xFFFF) << 0;        // Handler Low
        entry |= (uint64_t)0x08 << 16;               // Code Segment
        entry |= (uint64_t)(IDT_PRESENT | IDT_DPL0 | IDT_INTERRUPT) << 32;
        entry |= ((handlerAddr >> 16) & 0xFFFF) << 48; // Handler High
        
        return entry;
    }

    void loadIDT() {
        struct {
            uint16_t limit;
            uint64_t base;
        } __attribute__((packed)) IDTR;

        IDTR.limit = (idt.size() * 8) - 1;
        IDTR.base = reinterpret_cast(&idt[0]);
        
        __asm__ volatile("lidt %0" : : "m"(IDTR));
    }
    
    static inline void outportb(uint16_t port, uint8_t value) {
        __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
    }
};

} // namespace Kernel

#endif
void(void*)>