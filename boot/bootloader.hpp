#pragma once
#ifndef BOOTLOADER_HPP
#define BOOTLOADER_HPP

#include 
#include 
#include 
#include 
#include 
#include 
#include 
#include 

#include "../arch/x86_64/multiboot.hpp"
#include "../include/types.hpp"
#include "../arch/x86_64/cpu.hpp"
#include "../memory/MemoryManager.hpp"
#include "../memory/PageAllocator.hpp"
#include "../memory/vmm.hpp"

namespace Kernel {

class Bootloader {
public:
    enum class BootStage {
        PRE_INIT,
        HARDWARE_DETECTION,
        KERNEL_LOAD,
        FILESYSTEM_MOUNT,
        MEMORY_INIT,
        BOOT_SCREEN,
        KERNEL_STARTUP,
        COMPLETE
    };

    static Bootloader& getInstance() {
        static Bootloader instance;
        return instance;
    }

    Bootloader(const Bootloader&) = delete;
    Bootloader& operator=(const Bootloader&) = delete;

    void init(multiboot_info_t* mbi);
    bool initialize();
    bool detectHardware();
    bool loadKernel();
    bool mountFilesystems();
    bool initializeMemory();
    bool startKernel();
  
    BootStage getCurrentStage() const { return currentStage_; }
    bool isBootComplete() const { return currentStage_ == BootStage::COMPLETE; }

    void detectCPU();
    void detectMemoryCapacity();
    void probeHardwareInterfaces();
    void initializeInterruptControllers();

private:
    Bootloader() = default;

    void setupGDT();
    void setupIDT();
    void enablePaging();
    void initializeMemory(multiboot_info_t* mbi);
    void loadGDT();
    void loadIDT();
    void detectMemory(multiboot_info_t* mbi);
    void setupMemoryMap();
    void enterProtectedMode();

    struct BootState {
        bool gdtInitialized = false;
        bool idtInitialized = false;
        bool memoryInitialized = false;
        bool pagingEnabled = false;
    } bootState;

    struct HardwareState {
        bool cpuDetected = false;
        bool memoryDetected = false;
        uint64_t totalMemory = 0;
        uint32_t cpuCores = 0;
        std::string cpuVendor;
        std::string cpuModel;
    } hardwareState;

    struct GDTEntry {
        uint32_t base;
        uint32_t limit;
        uint8_t access;

        GDTEntry(uint32_t base = 0, uint32_t limit = 0, uint8_t access = 0)
            : base(base), limit(limit), access(access) {}
    };

    std::array globalDescriptorTable;
    BootStage currentStage_ = BootStage::PRE_INIT;
    std::vector detectedDevices_;
    std::filesystem::path kernelPath_ = "/boot/vmlinuz";
    std::filesystem::path initrdPath_ = "/boot/initrd.img";
    
    void logBootStage(const std::string& message);
    void handleBootError(const std::string& errorMessage);
    bool validateKernelImage(const std::filesystem::path& kernelPath);
};

} // namespace Kernel

#endif // BOOTLOADER_HPP