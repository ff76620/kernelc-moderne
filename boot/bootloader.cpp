#include "kernel/boot/bootloader.hpp"
#include "kernel/boot/bootscreen.hpp"
#include "kernel/core/kernel.hpp"

namespace Kernel {

void Bootloader::init(multiboot_info_t* mbi) {
    CPU::disableInterrupts();
    
    setupGDT();
    setupIDT();
    initializeMemory(mbi);
    enablePaging();
    enterProtectedMode();
    this->mbi = mbi;

    Kernel::start();
    
    CPU::enableInterrupts();
}

bool Bootloader::initialize() {
    try {
        currentStage_ = BootStage::PRE_INIT;
        logBootStage("Pre-initialization starting");
        
        if (!std::filesystem::exists("/boot")) {
            handleBootError("Boot directory not found");
            return false;
        }
        
        currentStage_ = BootStage::HARDWARE_DETECTION;
        return detectHardware();
    } catch (const std::exception& e) {
        handleBootError("Initialization failed: " + std::string(e.what()));
        return false;
    }
}

bool Bootloader::detectHardware() {
    logBootStage("Performing comprehensive hardware detection");
    
    try {
        detectCPU();
        detectMemoryCapacity();
        probeHardwareInterfaces();
        initializeInterruptControllers();
        
        detectedDevices_ = {
            "CPU: " + hardwareState.cpuVendor + " " + hardwareState.cpuModel,
            "Cores: " + std::to_string(hardwareState.cpuCores),
            "Memory: " + std::to_string(hardwareState.totalMemory / (1024 * 1024)) + " MB"
        };
        
        currentStage_ = BootStage::KERNEL_LOAD;
        return loadKernel();
    } catch (const std::exception& e) {
        handleBootError("Hardware detection failed: " + std::string(e.what()));
        return false;
    }
}

bool Bootloader::loadKernel() {
    logBootStage("Loading kernel");
    
    try {
        if (!std::filesystem::exists(kernelPath_)) {
            handleBootError("Kernel not found at " + kernelPath_.string());
            return false;
        }
        
        if (!validateKernelImage(kernelPath_)) {
            handleBootError("Kernel validation failed");
            return false;
        }
        
        currentStage_ = BootStage::FILESYSTEM_MOUNT;
        return mountFilesystems();
    } catch (const std::filesystem::filesystem_error& e) {
        handleBootError("Kernel loading error: " + std::string(e.what()));
        return false;
    }
}

bool Bootloader::mountFilesystems() {
    logBootStage("Mounting filesystems");
    
    try {
        std::filesystem::create_directories("/mnt/root");
        std::filesystem::create_directories("/mnt/home");
        
        currentStage_ = BootStage::MEMORY_INIT;
        return initializeMemory();
    } catch (const std::exception& e) {
        handleBootError("Filesystem mounting failed: " + std::string(e.what()));
        return false;
    }
}

bool Bootloader::initializeMemory() {
    logBootStage("Initializing system memory");
    currentStage_ = BootStage::BOOT_SCREEN;
    return true;
}

bool Bootloader::startKernel() {
    logBootStage("Starting kernel");
    currentStage_ = BootStage::COMPLETE;
    return true;
}

void Bootloader::detectCPU() {
  // Use CPUID instruction to get CPU info
  unsigned int eax, ebx, ecx, edx;
  unsigned int max_cpuid;

  // Get max CPUID level and vendor ID
  __asm__ volatile(
    "cpuid"
    : "=a"(max_cpuid), "=b"(ebx), "=c"(ecx), "=d"(edx)
    : "a"(0)  
  );

  // Store vendor string 
  char vendor[13];
  memcpy(vendor, &ebx, 4);
  memcpy(vendor+4, &edx, 4);
  memcpy(vendor+8, &ecx, 4);
  vendor[12] = '\0';
  hardwareState.cpuVendor = std::string(vendor);

  // Get processor brand string
  char brand[49];
  for(unsigned int i = 0x80000002; i <= 0x80000004; i++) {
    __asm__ volatile(
      "cpuid"
      : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
      : "a"(i)
    );
    
    memcpy(brand + (i-0x80000002)*16 + 0, &eax, 4);
    memcpy(brand + (i-0x80000002)*16 + 4, &ebx, 4);
    memcpy(brand + (i-0x80000002)*16 + 8, &ecx, 4);
    memcpy(brand + (i-0x80000002)*16 + 12, &edx, 4);
  }
  brand[48] = '\0';
  hardwareState.cpuModel = std::string(brand);

  // Get CPU features
  __asm__ volatile(
    "cpuid"
    : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
    : "a"(1)
  );

  // Extract core count from ebx  
  hardwareState.cpuCores = ((ebx >> 16) & 0xFF);

  // Check for long mode support
  bool hasLongMode = false;
  __asm__ volatile(
    "cpuid"
    : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
    : "a"(0x80000001)
  );
  hasLongMode = (edx >> 29) & 1;

  if (!hasLongMode) {
    handleBootError("CPU does not support 64-bit long mode");
    return;
  }

  // Store CPU features
  cpuFeatures.hasSSE = (edx >> 25) & 1;
  cpuFeatures.hasSSE2 = (edx >> 26) & 1; 
  cpuFeatures.hasAVX = (ecx >> 28) & 1;
  cpuFeatures.hasVMX = (ecx >> 5) & 1;
  cpuFeatures.hasSVM = (ecx >> 2) & 1;
  cpuFeatures.hasAES = (ecx >> 25) & 1;
  cpuFeatures.hasFPU = (edx >> 0) & 1;
  cpuFeatures.hasTSC = (edx >> 4) & 1;
  cpuFeatures.hasMSR = (edx >> 5) & 1;
  cpuFeatures.hasCX8 = (edx >> 8) & 1;
  cpuFeatures.hasCMOV = (edx >> 15) & 1;
  cpuFeatures.hasCLFSH = (edx >> 19) & 1;
  cpuFeatures.hasMMX = (edx >> 23) & 1;
  cpuFeatures.hasFXSR = (edx >> 24) & 1;
  cpuFeatures.hasXSAVE = (ecx >> 26) & 1;
  cpuFeatures.hasPCID = (ecx >> 17) & 1;
  cpuFeatures.hasOSXSAVE = (ecx >> 27) & 1;

  // Log additional CPU features
  logBootStage("CPU Features:");
  logBootStage(" - SSE: " + std::string(cpuFeatures.hasSSE ? "Yes" : "No"));
  logBootStage(" - SSE2: " + std::string(cpuFeatures.hasSSE2 ? "Yes" : "No")); 
  logBootStage(" - AVX: " + std::string(cpuFeatures.hasAVX ? "Yes" : "No"));
  logBootStage("Additional CPU Features:");
  logBootStage(" - VMX/SVM: " + std::string(cpuFeatures.hasVMX || cpuFeatures.hasSVM ? "Yes" : "No"));
  logBootStage(" - AES: " + std::string(cpuFeatures.hasAES ? "Yes" : "No"));
  logBootStage(" - TSC: " + std::string(cpuFeatures.hasTSC ? "Yes" : "No")); 
  logBootStage(" - MSR: " + std::string(cpuFeatures.hasMSR ? "Yes" : "No"));
  logBootStage(" - XSAVE: " + std::string(cpuFeatures.hasXSAVE ? "Yes" : "No"));
  logBootStage(" - PCID: " + std::string(cpuFeatures.hasPCID ? "Yes" : "No"));

  hardwareState.cpuDetected = true;
}

void Bootloader::detectMemoryCapacity() {
  // Get memory map from multiboot info
  if (!(mbi->flags & MULTIBOOT_INFO_MEMORY)) {
    handleBootError("No memory information from bootloader");
    return;
  }

  // Get total memory size
  hardwareState.totalMemory = mbi->mem_upper * 1024;

  // Parse memory map entries
  if (mbi->flags & MULTIBOOT_INFO_MEM_MAP) {
    multiboot_memory_map_t* mmap = (multiboot_memory_map_t*)mbi->mmap_addr;
    
    uint64_t usableMemory = 0;
    uint64_t reservedMemory = 0;
    uint64_t acpiMemory = 0;

    while((unsigned long)mmap < mbi->mmap_addr + mbi->mmap_length) {
      switch(mmap->type) {
        case MULTIBOOT_MEMORY_AVAILABLE:
          usableMemory += mmap->len;
          // Add to physical memory manager
          PhysicalMemoryManager::addFreeRegion(
            mmap->addr,
            mmap->addr + mmap->len
          );
          break;
          
        case MULTIBOOT_MEMORY_RESERVED:
          reservedMemory += mmap->len;
          break;
          
        case MULTIBOOT_MEMORY_ACPI_RECLAIMABLE:
          acpiMemory += mmap->len;
          break;
      }
      
      mmap = (multiboot_memory_map_t*)((unsigned long)mmap + mmap->size + sizeof(mmap->size));
    }

    // Log memory info
    logBootStage("Memory detected: " + 
      std::to_string(usableMemory/1024/1024) + "MB usable, " +
      std::to_string(reservedMemory/1024/1024) + "MB reserved, " +
      std::to_string(acpiMemory/1024/1024) + "MB ACPI reclaimable");
  }

  hardwareState.memoryDetected = true;
}

void Bootloader::setupGDT() {
    // Create GDT entries
    globalDescriptorTable[0] = GDTEntry(); // Null descriptor
    globalDescriptorTable[1] = GDTEntry(0, 0xfffff, 0x9A); // Code segment
    globalDescriptorTable[2] = GDTEntry(0, 0xfffff, 0x92); // Data segment
    globalDescriptorTable[3] = GDTEntry(0, 0xfffff, 0xFA); // User code segment
    globalDescriptorTable[4] = GDTEntry(0, 0xfffff, 0xF2); // User data segment

    loadGDT();
    bootState.gdtInitialized = true;
}

void Bootloader::setupIDT() {
    // Keep advanced IDT setup from original implementation
    loadIDT();
    bootState.idtInitialized = true;
}

void Bootloader::enablePaging() {
    // Configure paging
    PageTable* pageTable = new PageTable();
    pageTable->initialize();
    
    // Enable paging
    VMM::enablePaging();
    
    bootState.pagingEnabled = true;
}

void Bootloader::initializeMemory(multiboot_info_t* mbi) {
    // Initialize basic memory management
    MemoryManager::initialize(mbi->mem_upper);
    
    // Initialize advanced memory features
    detectMemory(mbi);
    setupMemoryMap();
    PageAllocator::init();
    VirtualMemoryManager::init();
    
    bootState.memoryInitialized = true;
}

void Bootloader::detectMemory(multiboot_info_t* mbi) {
    if(!(mbi->flags & MULTIBOOT_INFO_MEMORY)) {
        handleBootError("No memory information provided by bootloader");
        return;
    }

    hardwareState.totalMemory = mbi->mem_upper * 1024;
    
    if(mbi->flags & MULTIBOOT_INFO_MEM_MAP) {
        multiboot_memory_map_t* mmap = (multiboot_memory_map_t*)mbi->mmap_addr;
        while((unsigned long)mmap < mbi->mmap_addr + mbi->mmap_length) {
            if(mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
                PhysicalMemoryManager::addFreeRegion(
                    mmap->addr,
                    mmap->addr + mmap->len
                );
            }
            mmap = (multiboot_memory_map_t*)((unsigned long)mmap + mmap->size + sizeof(mmap->size));
        }
    }

    memoryMap.analyze(mbi);
    hardwareState.memoryDetected = true;
}

void Bootloader::enterProtectedMode() {
    asm volatile("cli");
    // Enable protected mode bit in CR0
    asm volatile(
        "mov %%cr0, %%eax\n"
        "or $1, %%eax\n"
        "mov %%eax, %%cr0"
        : : : "eax"
    );

    // Long jump to reload CS with 32-bit segment
    asm volatile(
        "ljmp $0x08, $1f\n"
        "1:\n"
    );

    // Set up segment registers
    asm volatile(
        "mov $0x10, %ax\n"
        "mov %ax, %ds\n"
        "mov %ax, %es\n"
        "mov %ax, %fs\n"
        "mov %ax, %gs\n"
        "mov %ax, %ss\n"
    );
}

void Bootloader::probeHardwareInterfaces() {
  // Probe PCI devices
  for(uint8_t bus = 0; bus < 256; bus++) {
    for(uint8_t device = 0; device < 32; device++) {
      for(uint8_t function = 0; function < 8; function++) {
        uint32_t id = pciConfigRead(bus, device, function, 0);
        if(id != 0xFFFFFFFF) {
          uint32_t classCode = pciConfigRead(bus, device, function, 8);
          uint8_t baseClass = (classCode >> 24) & 0xFF;
          uint8_t subClass = (classCode >> 16) & 0xFF;
          
          // Store device info
          PCIDevice dev;
          dev.bus = bus;
          dev.device = device;
          dev.function = function;
          dev.vendorId = id & 0xFFFF;
          dev.deviceId = (id >> 16) & 0xFFFF;
          dev.classCode = baseClass;
          dev.subClass = subClass;
          
          detectedDevices_.push_back(dev);
        }
      }
    }
  }

  // Probe legacy devices
  probeLegacyInterrupts();
  probeIOPorts(); 
}

void Bootloader::initializeInterruptControllers() {
  // Initialize 8259 PIC
  // Remap IRQs to avoid conflicts with CPU exceptions
  outb(0x20, 0x11); // Initialize PIC1
  outb(0xA0, 0x11); // Initialize PIC2
  
  outb(0x21, 0x20); // Map PIC1 to 0x20-0x27
  outb(0xA1, 0x28); // Map PIC2 to 0x28-0x2F
  
  outb(0x21, 0x04); // Tell PIC1 about PIC2 at IRQ2
  outb(0xA1, 0x02); // Tell PIC2 its cascade identity
  
  outb(0x21, 0x01); // Set 8086 mode
  outb(0xA1, 0x01);

  // Mask all interrupts initially
  outb(0x21, 0xFF);
  outb(0xA1, 0xFF);

  // Initialize APIC if available
  if(cpuHasAPIC()) {
    // Disable PIC
    outb(0xA1, 0xFF);
    outb(0x21, 0xFF);

    // Enable APIC
    uint64_t apicBase = rdmsr(IA32_APIC_BASE_MSR);
    apicBase |= (1 << 11); // Set enable bit
    wrmsr(IA32_APIC_BASE_MSR, apicBase);

    // Set up Local APIC
    volatile uint32_t* lapic = (uint32_t*)0xFEE00000;
    lapic[0xF0/4] = 0x1FF; // Enable spurious interrupts
  }
}

void Bootloader::logBootStage(const std::string& message) {
    std::cout << "[BOOT] " << message << std::endl;
}

void Bootloader::handleBootError(const std::string& errorMessage) {
    std::cerr << "[BOOT ERROR] " << errorMessage << std::endl;
}

bool Bootloader::validateKernelImage(const std::filesystem::path& kernelPath) {
  // Open kernel file
  std::ifstream kernel(kernelPath, std::ios::binary);
  if(!kernel) {
    handleBootError("Failed to open kernel image");
    return false;
  }

  // Read ELF header
  Elf64_Ehdr header;
  kernel.read((char*)&header, sizeof(header));
  
  // Verify ELF magic number
  if(header.e_ident[EI_MAG0] != ELFMAG0 ||
     header.e_ident[EI_MAG1] != ELFMAG1 ||
     header.e_ident[EI_MAG2] != ELFMAG2 ||
     header.e_ident[EI_MAG3] != ELFMAG3) {
    handleBootError("Invalid kernel ELF magic number");
    return false;
  }

  // Verify architecture
  if(header.e_machine != EM_X86_64) {
    handleBootError("Kernel architecture mismatch");
    return false;
  }

  // Calculate checksum
  uint32_t checksum = 0;
  char buffer[4096];
  while(kernel.read(buffer, sizeof(buffer))) {
    for(size_t i = 0; i < kernel.gcount(); i++) {
      checksum += (unsigned char)buffer[i];
    }
  }

  // Verify against embedded checksum
  if(checksum != header.e_checksum) {
    handleBootError("Kernel checksum mismatch");
    return false;
  }

  return true;
}

} // namespace Kernel