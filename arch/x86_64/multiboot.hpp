#ifndef MULTIBOOT_HPP
#define MULTIBOOT_HPP

#include 

namespace Boot {

#define MULTIBOOT2_MAGIC 0x36d76289

struct MultibootTag {
    uint32_t type;
    uint32_t size;
};

struct MultibootInfo {
    uint32_t total_size;
    uint32_t reserved;
    MultibootTag tags[];
};

enum MultibootTagType {
    MULTIBOOT_TAG_END = 0,
    MULTIBOOT_TAG_CMDLINE = 1,
    MULTIBOOT_TAG_BOOT_LOADER_NAME = 2,
    MULTIBOOT_TAG_MODULE = 3,
    MULTIBOOT_TAG_MEMORY = 4,
    MULTIBOOT_TAG_BOOTDEV = 5,
    MULTIBOOT_TAG_MMAP = 6,
    MULTIBOOT_TAG_FRAMEBUFFER = 8
};

struct MemoryMapEntry {
    uint64_t baseAddr;
    uint64_t length;
    uint32_t type;
    uint32_t reserved;
};

class MultibootParser {
private:
    MultibootInfo* mbi;
    
public:
    MultibootParser(void* multiboot_ptr);
    
    const char* getCommandLine();
    const char* getBootloaderName();
    uint64_t getMemorySize();
    MemoryMapEntry* getMemoryMap(size_t* count);
    void* getModule(const char* name, size_t* size);
    
    void parseFramebufferInfo(uint32_t* width, uint32_t* height, uint32_t* bpp);
};

} // namespace Boot

#endif