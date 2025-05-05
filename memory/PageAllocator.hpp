#ifndef PAGE_ALLOCATOR_HPP
#define PAGE_ALLOCATOR_HPP

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

class PageAllocator {
private:
    PageBitmap bitmap;
    void* memoryStart;
    size_t totalPages;
    size_t freePages;
    std::mutex allocationMutex;
    std::vector freePageList;
    
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
    
    PageAllocator() : memoryStart(nullptr), totalPages(0), freePages(0) {}
    
    void initialize(void* start, size_t memorySize) {
        std::lock_guard lock(allocationMutex);
        memoryStart = start;
        totalPages = memorySize / PAGE_SIZE;
        freePages = totalPages;
        
        // Initialize bitmap
        bitmap.size = (totalPages + 7) / 8;
        bitmap.buffer = new uint8_t[bitmap.size]();
    }
    
    void* allocate(size_t size, bool isKernel = false) {
        std::lock_guard lock(allocationMutex);
        size_t pageCount = (size + PAGE_SIZE - 1) / PAGE_SIZE;
        
        // Find continuous free pages
        size_t consecutivePages = 0;
        size_t startPage = 0;
        
        for (size_t i = 0; i < totalPages; i++) {
            if (!getBit(i)) {
                if (consecutivePages == 0) startPage = i;
                consecutivePages++;
                
                if (consecutivePages == pageCount) {
                    // Mark pages as used
                    for (size_t j = 0; j < pageCount; j++) {
                        setBit(startPage + j, true);
                    }
                    freePages -= pageCount;
                    
                    return (void*)((uintptr_t)memoryStart + startPage * PAGE_SIZE);
                }
            } else {
                consecutivePages = 0;
            }
        }
        
        return nullptr; // No sufficient continuous pages found
    }
    
    void free(void* address, size_t size) {
        std::lock_guard lock(allocationMutex);
        size_t pageCount = (size + PAGE_SIZE - 1) / PAGE_SIZE;
        size_t startPage = getPageIndex(address);
        
        for (size_t i = 0; i < pageCount; i++) {
            setBit(startPage + i, false);
        }
        freePages += pageCount;
    }
    
    size_t getFreePageCount() const { return freePages; }
    size_t getTotalPageCount() const { return totalPages; }
    
    float getFragmentationRatio() {
        size_t fragments = 0;
        bool inFragment = false;
        
        for (size_t i = 0; i < totalPages; i++) {
            if (!getBit(i) && !inFragment) {
                fragments++;
                inFragment = true;
            } else if (getBit(i)) {
                inFragment = false;
            }
        }
        
        return static_cast<float w-tid="18">(fragments) / static_cast<float w-tid="19">(freePages);
    }
    
    ~PageAllocator() {
        delete[] bitmap.buffer;
    }
};

} // namespace Memory

#endif