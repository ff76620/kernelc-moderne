#include "kernel/memory/MemoryManager.hpp"
#include "kernel/loggin/EventLogger.hpp"
#include "kernel/memory/PageAllocator.hpp" 
#include "kernel/memory/vmm.hpp"
#include 
#include 
#include 
#include 
#include 
#include 

namespace Kernel {
namespace Memory {

class OptimizedUnifiedMemoryManager : public MemoryManager {
private:
    static constexpr size_t PAGE_SIZE = 4096;
    static constexpr size_t CACHE_LINE_SIZE = 64;
    static constexpr size_t TLB_SIZE = 256;
    
    struct TLBEntry {
        uint64_t virtualAddress;
        uint64_t physicalAddress;
        bool valid;
    };
    
    struct MemoryPool {
        void* start;
        size_t size;
        std::atomic used{0};
        WorkloadType optimizedFor;
        FragmentationMetrics fragmentationMetrics;
    };

    struct PoolAllocationInfo {
      size_t poolIndex;
      size_t size;
    };

    struct PageAllocationInfo {
      size_t numPages;
    };

    struct FragmentationMetrics {
      std::atomic totalAllocations{0};
      std::atomic totalDeallocations{0};
      std::atomic fragmentedSpace{0};
      
      void recordDeallocation(size_t size) {
        totalDeallocations++;
        fragmentedSpace += size;
      }
      
      bool shouldDefragment() const {
        return fragmentedSpace > 1024 * 1024; // 1MB
      }
    };

    std::array tlbCache;
    std::array pools;
    std::unique_ptr pageAllocator;
    std::unique_ptr compression;
    std::unique_ptr profiler;
    VirtualMemoryManager& vmm;
    std::mutex mutex;
    std::unordered_map pageTable;
    std::unordered_map poolAllocations;
    std::unordered_map pageAllocations;
    PhysicalPagePool physicalPagePool;
    
    size_t totalMemory;
    size_t availableMemory;

public:
    OptimizedUnifiedMemoryManager() 
        : vmm(VirtualMemoryManager::getInstance())
        , totalMemory(0)
        , availableMemory(0) {
        initialize();
    }

    bool initialize() override {
        EventLogger::log("Initializing Optimized Unified Memory Manager...");
        
        try {
            setupMemorySubsystems();
            setupPaging();
            setupGameProfiles();
            configureMemoryPools();
            initializeTLB();
            
            totalMemory = pageAllocator->getTotalPages() * PAGE_SIZE;
            availableMemory = pageAllocator->getAvailablePages() * PAGE_SIZE;
            
            EventLogger::log("Memory Manager initialized successfully");
            return true;
        }
        catch (const std::exception& e) {
            EventLogger::log("Error: " + std::string(e.what()));
            return false;
        }
    }

    void* allocateMemory(size_t size) {
        if (size == 0) return nullptr;
        
        std::lock_guard lock(mutex);
        
        // Cache-line alignment
        size = (size + CACHE_LINE_SIZE - 1) & ~(CACHE_LINE_SIZE - 1);
        
        // Workload detection
        auto workloadType = profiler->detectWorkloadType();
        
        // Try optimized pool first
        for (auto& pool : pools) {
            if (pool.optimizedFor == workloadType && pool.size >= size) {
                size_t offset = pool.used.fetch_add(size);
                if (offset + size <= pool.size) {
                    void* ptr = static_cast(pool.start) + offset;
                    poolAllocations[ptr] = {&pool - &pools[0], size};
                    return ptr;
                }
                pool.used.fetch_sub(size);
            }
        }

        // Fallback to page allocation
        size_t pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;
        void* virtualAddr = pageAllocator->allocatePages(pages);
        
        if (!virtualAddr) return nullptr;

        for (size_t i = 0; i < pages; i++) {
            uint64_t vAddr = reinterpret_cast(virtualAddr) + i * PAGE_SIZE;
            uint64_t pAddr = allocatePhysicalPage();
            pageTable[vAddr] = pAddr;
            updateTLB(vAddr, pAddr);
        }

        pageAllocations[reinterpret_cast(virtualAddr)] = {pages};
        
        return virtualAddr;
    }

    bool freeMemory(void* ptr) {
        std::lock_guard lock(mutex);

        // Start by checking if this is a pool allocation
        auto poolIt = poolAllocations.find(ptr); 
        if (poolIt != poolAllocations.end()) {
            MemoryPool& pool = pools[poolIt->second.poolIndex];
            
            // Track pool statistics
            pool.used.fetch_sub(poolIt->second.size);
            pool.fragmentationMetrics.recordDeallocation(poolIt->second.size);
            
            // Clear pool allocation tracking
            poolAllocations.erase(poolIt);
            
            // Trigger pool defragmentation if needed
            if (pool.fragmentationMetrics.shouldDefragment()) {
                defragmentPool(pool);
            }
            
            EventLogger::log("Pool memory freed", {
              {"pool_index", poolIt->second.poolIndex},
              {"size", poolIt->second.size},
              {"address", fmt::ptr(ptr)}
            });
            
            return true;
        }

        // Handle page-based allocations
        uint64_t vAddr = reinterpret_cast(ptr);
        auto pageRange = pageAllocations.find(vAddr);
        
        if (pageRange == pageAllocations.end()) {
            EventLogger::logError("Invalid free - address not found", {
              {"address", fmt::ptr(ptr)} 
            });
            return false;
        }

        // Clear TLB entries for all pages
        const size_t numPages = pageRange->second.numPages;
        for (size_t i = 0; i < numPages; i++) {
            uint64_t pageAddr = vAddr + (i * PAGE_SIZE);
            invalidateTLBEntry(pageAddr);
            
            // Free physical pages
            auto physPage = pageTable.find(pageAddr);
            if (physPage != pageTable.end()) {
                physicalPagePool.freePage(physPage->second);
                pageTable.erase(physPage);
            }
        }

        // Return pages to page allocator
        pageAllocator->freePages(ptr, numPages);
        pageAllocations.erase(pageRange);

        // Update memory stats
        availableMemory += (numPages * PAGE_SIZE);
        
        EventLogger::log("Page memory freed", {
          {"address", fmt::ptr(ptr)},
          {"pages", numPages}, 
          {"size", numPages * PAGE_SIZE}
        });

        return true;
    }

private:
    void setupMemorySubsystems() {
        compression = std::make_unique();
        profiler = std::make_unique();
        pageAllocator = std::make_unique();
        
        compression->enableAdvancedZRAM();
        profiler->enableAutoDetection();
        pageAllocator->enableDynamicHugePages();
    }

    void configureMemoryPools() {
        const size_t sizes[] = {64, 256, 1024, 4096};
        const WorkloadType types[] = {
            WorkloadType::GAMING,
            WorkloadType::CREATIVE,
            WorkloadType::GENERAL,
            WorkloadType::HIGH_PERFORMANCE
        };
        
        for(size_t i = 0; i < pools.size(); ++i) {
            pools[i].size = sizes[i] * 1024;
            pools[i].start = pageAllocator->allocatePages(pools[i].size / PAGE_SIZE);
            pools[i].optimizedFor = types[i];
        }
    }

    void setupGameProfiles() {
        std::vector profiles = {
            {"ModernGames", 2048_MB, true, Priority::HIGH},
            {"CreativeApps", 4096_MB, true, Priority::HIGH},
            {"LegacyGames", 1024_MB, true, Priority::NORMAL}
        };

        for (const auto& profile : profiles) {
            profiler->addGameProfile(profile);
        }
    }

    void updateTLB(uint64_t virtualAddr, uint64_t physicalAddr) {
        size_t index = virtualAddr % TLB_SIZE;
        tlbCache[index] = {virtualAddr, physicalAddr, true};
    }

    void invalidateTLBEntry(uint64_t virtualAddr) {
        size_t index = virtualAddr % TLB_SIZE;
        tlbCache[index].valid = false;
    }

    void defragmentPool(MemoryPool& pool) {
        std::lock_guard defragLock(pool.defragMutex);
        
        // Only defrag if still needed after acquiring lock
        if (!pool.fragmentationMetrics.shouldDefragment()) {
            return;
        }

        // Copy live allocations
        std::vector> liveAllocations;
        for (const auto& [ptr, info] : poolAllocations) {
            if (info.poolIndex == &pool - &pools[0]) {
                liveAllocations.emplace_back(ptr, info.size); 
            }
        }

        // Sort by address
        std::sort(liveAllocations.begin(), liveAllocations.end());

        // Compact allocations
        char* dest = static_cast(pool.start);
        for (const auto& [ptr, size] : liveAllocations) {
            if (ptr != dest) {
                std::memcpy(dest, ptr, size);
                // Update allocation tracking
                poolAllocations[dest] = {&pool - &pools[0], size};
                poolAllocations.erase(ptr);
            }
            dest += size;
        }

        // Reset fragmentation metrics
        pool.fragmentationMetrics = FragmentationMetrics{};
        pool.used.store(dest - static_cast(pool.start));

        EventLogger::log("Pool defragmented", {
          {"pool_index", &pool - &pools[0]},
          {"compacted_size", pool.used.load()}
        });
    }
};

} // namespace Memory
} // namespace Kernel