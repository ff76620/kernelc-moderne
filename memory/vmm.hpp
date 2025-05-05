#ifndef VMM_HPP
#define VMM_HPP

#include <chrono>
#include <mutex>
#include <unordered_map>
#include <vector>
#include "../include/types.hpp"

namespace Memory {

class CompressionEngine {
private:
    std::array<TLBEntry, 1024> tlb_entries;
    std::vector<NumaNode> nodes;
    std::vector<CompressedPage> compressed_pages;

public:
    void compressPage(void* page) {
        // Advanced compression using LZ4
        uint8_t* data = static_cast<uint8_t*>(page);
        auto compressed = lz4_compress(data);
        compressed_pages.push_back(compressed);
        
        // Backup RLE compression if needed
        if (compressed.size() >= 4096) {
            std::vector<uint8_t> rle_compressed;
            for(size_t i = 0; i < 4096;) {
                uint8_t count = 1;
                uint8_t current = data[i];
                while(i + count < 4096 && data[i + count] == current && count < 255) {
                    count++;
                }
                rle_compressed.push_back(count);
                rle_compressed.push_back(current);
                i += count;
            }
            if(rle_compressed.size() < 4096) {
                std::memcpy(page, rle_compressed.data(), rle_compressed.size());
                std::memset(static_cast<uint8_t*>(page) + rle_compressed.size(), 0, 
                           4096 - rle_compressed.size());
            }
        }
    }

    void flushTLB() {
        asm volatile("invlpg (%0)" : : "r"(addr) : "memory");
    }

    void balanceNumaMemory() {
        for(auto& node : nodes) {
            if(node.getUsage() > threshold) {
                migratePages(node, findLeastLoadedNode());
            }
        }
    }
};

struct PageTableEntry {
    uint64_t present : 1;
    uint64_t writable : 1;
    uint64_t userAccess : 1;
    uint64_t writeThrough : 1;
    uint64_t cacheDisable : 1;
    uint64_t accessed : 1;
    uint64_t dirty : 1;
    uint64_t pageSize : 1;
    uint64_t global : 1;
    uint64_t available : 3;
    uint64_t address : 40;
    uint64_t reserved : 11;
    uint64_t noExecute : 1;
};

class VirtualMemoryManager {
private:
    struct PageInfo {
        std::chrono::steady_clock::time_point lastAccessed;
        uint32_t accessCount;
        bool isCompressed;
        bool isHugePage;
        uint8_t numaNode;
    };
    
    struct MemoryBlock {
        void* address;
        size_t size;
        bool isFree;
        uint32_t flags;
        uint8_t protectionBits;
    };

    std::unordered_map<void*, PageInfo> pageTracker;
    PageTableEntry* pml4;
    void* kernelVirtualBase;
    std::vector<MemoryBlock> memoryBlocks;
    std::mutex memoryMutex;
    std::unique_ptr<CompressionEngine> compressionEngine;
    
    const size_t PAGE_SIZE = 4096;
    const size_t HUGE_PAGE_SIZE = 2 * 1024 * 1024; // 2MB
    const size_t INACTIVE_THRESHOLD = 5000; // milliseconds
    size_t totalMemory;
    size_t compressedMemory;

public:
    void initialize() {
        compressionEngine = std::make_unique<CompressionEngine>();
        
        // Initialize PML4 with protection
        pml4 = static_cast<PageTableEntry*>(aligned_alloc(4096, 4096));
        memset(pml4, 0, 4096);
        
        // Map kernel space with security features
        kernelVirtualBase = reinterpret_cast<void*>(0xFFFF800000000000);
        for(size_t i = 0; i < 512; i++) {
            pml4[i].present = 1;
            pml4[i].writable = 1;
            pml4[i].userAccess = 0;
            pml4[i].writeThrough = 0;
            pml4[i].cacheDisable = 0;
            pml4[i].noExecute = 1; // Security: NX bit
            pml4[i].address = i * 0x40000000;
        }
        
        initializeNUMA();
        setupHugePages();
        initializeSwap();
    }

    void* allocateVirtualMemory(size_t size, uint32_t flags) {
        std::lock_guard<std::mutex> lock(memoryMutex);
        
        // Try huge page allocation for large requests
        if (size >= HUGE_PAGE_SIZE && (flags & HUGE_PAGE_FLAG)) {
            void* hugePageAddr = allocateHugePage(size);
            if (hugePageAddr) return hugePageAddr;
        }
        
        size = alignToPage(size);
        
        // Try normal allocation with compression
        auto bestFit = findBestFitBlock(size);
        if(bestFit != memoryBlocks.end()) {
            void* address = bestFit->address;
            splitBlock(bestFit, size);
            trackPage(address, flags);
            return address;
        }
        
        // Compress inactive pages if needed
        if (compressedMemory < totalMemory * 0.3) { // Max 30% compression
            compressInactivePages();
            bestFit = findBestFitBlock(size);
            if(bestFit != memoryBlocks.end()) {
                void* address = bestFit->address;
                splitBlock(bestFit, size);
                trackPage(address, flags);
                return address;
            }
        }
        
        // Last resort: allocate new block
        void* newBlock = allocateNewBlock(size, flags);
        if(newBlock) {
            trackPage(newBlock, flags);
            balanceNumaMemory();
        }
        return newBlock;
    }

    void handlePageFault(VirtualAddress addr) {
        if(!isPresent(addr)) {
            if(isHugePage(addr)) {
                allocateHugePage(addr);
            } else {
                allocatePage(addr);
            }
        }
        if(isSwapped(addr)) {
            swapIn(addr);
        }
        if(isCompressed(addr)) {
            decompress(addr);
        }
        updateAccessStats(addr);
    }

private:
    // Enhanced memory management functions
    void trackPage(void* address, uint32_t flags) {
        pageTracker[address] = {
            std::chrono::steady_clock::now(),
            0,
            false,
            bool(flags & HUGE_PAGE_FLAG),
            getCurrentNumaNode()
        };
    }

    void balanceNumaMemory() {
        compressionEngine->balanceNumaMemory();
    }

    bool isInactivePage(void* address) {
        auto it = pageTracker.find(address);
        if(it == pageTracker.end()) return false;
        
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>
                       (now - it->second.lastAccessed);
        
        return duration.count() > INACTIVE_THRESHOLD && 
               it->second.accessCount < 100;
    }

    void compressInactivePages() {
        for(auto& block : memoryBlocks) {
            if(!block.isFree && isInactivePage(block.address)) {
                auto it = pageTracker.find(block.address);
                if(!it->second.isCompressed) {
                    compressionEngine->compressPage(block.address);
                    it->second.isCompressed = true;
                    compressedMemory += block.size;
                }
            }
        }
    }
};

} // namespace Memory

#endif // VMM_HPP