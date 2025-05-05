#ifndef ENHANCED_MEMORY_MANAGER_HPP
#define ENHANCED_MEMORY_MANAGER_HPP

#include 
#include 
#include 
#include 
#include 
#include 

namespace Memory {

struct PageBitmap {
    uint8_t* buffer;
    size_t size;
    
    PageBitmap() : buffer(nullptr), size(0) {}
};

class EnhancedPageAllocator {
private:
    PageBitmap bitmap;
    void* memoryStart;
    size_t totalPages;
    size_t freePages;
    std::mutex allocationMutex;
    std::vector<void* w-tid="16"> freePageList;
    
    size_t getPageIndex(void* address) {
        return ((uintptr_t)address - (uintptr_t)memoryStart) / PAGE_SIZE;
    }
    
    void setBit(size_t index, bool value) {
        size_t byteIndex = index / 8;
        size_t bitIndex = index % 8;
        uint8_t bitMask = 1 << bitIndex;
        
        if (value) {
            bitmap.buffer[byteIndex] |= bitMask;
        } else {
            bitmap.buffer[byteIndex] &= ~bitMask;
        }
    }
    
    bool getBit(size_t index) {
        size_t byteIndex = index / 8;
        size_t bitIndex = index % 8;
        uint8_t bitMask = 1 << bitIndex;
        return (bitmap.buffer[byteIndex] & bitMask) != 0;
    }

public:
    static const size_t PAGE_SIZE = 4096;
    static const size_t HUGE_PAGE_SIZE = 2 * 1024 * 1024;
    
    void initialize(void* start, size_t memorySize) {
        std::lock_guard lock(allocationMutex);
        memoryStart = start;
        totalPages = memorySize / PAGE_SIZE;
        freePages = totalPages;
        
        bitmap.size = (totalPages + 7) / 8;
        bitmap.buffer = new uint8_t[bitmap.size]();
    }
    
    void* allocate(size_t size, bool useHugePages = false) {
        std::lock_guard lock(allocationMutex);
        size_t pageSize = useHugePages ? HUGE_PAGE_SIZE : PAGE_SIZE;
        size_t pageCount = (size + pageSize - 1) / pageSize;
        
        size_t consecutivePages = 0;
        size_t startPage = 0;
        
        for (size_t i = 0; i < totalPages; i++) {
            if (!getBit(i)) {
                if (consecutivePages == 0) startPage = i;
                consecutivePages++;
                
                if (consecutivePages == pageCount) {
                    for (size_t j = 0; j < pageCount; j++) {
                        setBit(startPage + j, true);
                    }
                    freePages -= pageCount;
                    return (void*)((uintptr_t)memoryStart + startPage * pageSize);
                }
            } else {
                consecutivePages = 0;
            }
        }
        return nullptr;
    }
};

class EnhancedMemoryManager {
private:
    struct CacheConfig {
        size_t l1Size;
        size_t l2Size;
        size_t l3Size;
        bool prefetchEnabled;
        std::vector prefetchPattern;
    };

    struct GameMemoryConfig {
        size_t textureCache;
        size_t geometryCache;
        bool useHugePages;
    };

    static const size_t FAST_POOL_THRESHOLD = 4096;
    static const size_t LARGE_PAGE_THRESHOLD = 2 * 1024 * 1024;
    
    EnhancedPageAllocator pageAllocator;
    CacheConfig cacheConfig;
    GameMemoryConfig gameConfig;
    std::mutex mutex;
    std::map<void*, size_t="" w-tid="20"> allocations;
    std::atomic<size_t w-tid="21"> availableMemory;
    size_t totalMemory;
    
    std::vector<void* w-tid="22"> memoryPools;

public:
    static EnhancedMemoryManager& getInstance() {
        static EnhancedMemoryManager instance;
        return instance;
    }
    
    void initialize() {
        std::lock_guard lock(mutex);
        
        // Initialize with 16GB managed memory
        void* memoryStart = (void*)0x100000;
        size_t memorySize = 16ULL * 1024 * 1024 * 1024;
        
        pageAllocator.initialize(memoryStart, memorySize);
        totalMemory = memorySize;
        availableMemory = memorySize;
        
        initializeCache();
        initializeGameConfig();
    }
    
    void* allocateMemory(size_t size) {
        std::lock_guard lock(mutex);
        
        if (size >= LARGE_PAGE_THRESHOLD) {
            return pageAllocator.allocate(size, true);  // Use huge pages
        } else {
            return pageAllocator.allocate(size);
        }
    }
    
    float getMemoryUtilization() {
        return 1.0f - (static_cast<float w-tid="25">(availableMemory) / static_cast<float w-tid="26">(totalMemory));
    }

private:
    void initializeCache() {
        cacheConfig = {
            .l1Size = 32 * 1024,
            .l2Size = 256 * 1024,
            .l3Size = 8 * 1024 * 1024,
            .prefetchEnabled = true,
            .prefetchPattern = {1, 2, 4, 8}
        };
    }
    
    void initializeGameConfig() {
        gameConfig = {
            .textureCache = 512 * 1024 * 1024,
            .geometryCache = 256 * 1024 * 1024,
            .useHugePages = true
        };
    }
};

} // namespace Memory

#endif
float>float>void*>size_t>void*,>void*>