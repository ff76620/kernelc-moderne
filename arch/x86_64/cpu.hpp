#ifndef CPU_HPP
#define CPU_HPP

#include 

namespace Arch {
namespace x86_64 {

class CPU {
private:
    uint64_t cr3;  // Page table base register
    bool interruptsEnabled;
    
public:
    enum class PrivilegeLevel {
        Ring0 = 0, // Kernel
        Ring1 = 1, // Device drivers 
        Ring2 = 2, // Reserved
        Ring3 = 3  // User applications
    };

    void initialize();
    
    // Control registers
    void setCR3(uint64_t pml4Address);
    uint64_t getCR3() const;
    
    // Interrupt management  
    void enableInterrupts();
    void disableInterrupts();
    bool areInterruptsEnabled() const;
    
    // CPU features
    bool supportsNX() const;       // No Execute bit
    bool supportsSSE() const;      // SIMD Extensions
    bool supportsAVX() const;      // Advanced Vector Extensions
    bool supportsAPIC() const;     // Advanced Programmable Interrupt Controller
    
    // Cache control
    void invalidateCache();
    void invalidatePage(void* address);
    void flushTLB();
    
    // Privilege level
    PrivilegeLevel getCurrentPrivilegeLevel() const;
    void setPrivilegeLevel(PrivilegeLevel level);
    
    // CPU identification
    uint32_t getCPUID() const;
    const char* getVendorString() const;
    
    // Power management
    void halt();
    void shutdown();
};

}} // namespace Arch::x86_64

#endif