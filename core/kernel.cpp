
#include "kernel/core/kernel.hpp"
#include "kernel/drivers/DriverManager.hpp"
#include "kernel/process/ProcessManager.hpp"
#include "kernel/scheduler/scheduler.hpp"
#include "kernel/arch/x86_64/cpu.hpp"
#include "kernel/loggin/EventLogger.hpp"

namespace Kernel {

bool KernelSystem::initialized = false;
InterruptManager* KernelSystem::interruptManager = nullptr;
SchedulerSystem* KernelSystem::scheduler = nullptr;
VirtualMemoryManager* KernelSystem::memoryManager = nullptr;
DriverManager* KernelSystem::driverManager = nullptr;

void KernelSystem::initialize() {
    if(initialized) return;
    
    // Initialize core subsystems
    EventLogger::log("Initializing kernel subsystems...");
    
    // Setup memory management
    memoryManager = new VirtualMemoryManager();
    memoryManager->initialize();
    
    // Setup interrupt handling
    interruptManager = new InterruptManager();
    interruptManager->initialize();
    
    // Initialize scheduler
    scheduler = new SchedulerSystem();
    scheduler->initialize();
    
    // Setup device drivers
    driverManager = new DriverManager();
    driverManager->initialize();
    
    // Configuration du système de fichiers
    VFS::initialize();
    
    initialized = true;
    EventLogger::log("Kernel initialization complete");
}

void KernelSystem::start() {
    if(!initialized) {
        initialize();
    }
    
    EventLogger::log("Starting kernel services...");
    
    // Start all system services
    startSystemServices();
    
    // Enable interrupts and start scheduling
    CPU::enableInterrupts();
    scheduler->start();
    
    EventLogger::log("Kernel fully operational");
}

void KernelSystem::startSystemServices() {
    // Démarrage des services essentiels
    SecurityManager::start();
    NetworkStack::initialize();
    AudioSystem::initialize();
    
    // Configuration des pilotes graphiques
    GraphicsManager::initialize();
    
    // Start essential system services
    driverManager->startAll();
}

} // namespace Kernel