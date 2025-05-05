
            class PageAllocator {
            private:
                std::unordered_map allocations;
                
            public:
                void* AllocateAligned(size_t size, size_t alignment) {
                    void* ptr = _aligned_malloc(size, alignment);
                    if (ptr) {
                        allocations[ptr] = size;
                    }
                    return ptr;
                }
                
                void Deallocate(void* ptr) {
                    if (allocations.find(ptr) != allocations.end()) {
                        _aligned_free(ptr);
                        allocations.erase(ptr);
                    }
                }
            };

            class GraphicsResourceManager {
            private:
                struct Resource {
                    void* data;
                    size_t size;
                    std::string type;
                    bool isLocked;
                };
                
                std::unordered_map resources;
                
            public:
                bool AllocateResource(const std::string& name, size_t size, const std::string& type) {
                    void* data = _aligned_malloc(size, 16);
                    if (!data) return false;
                    
                    resources[name] = {data, size, type, false};
                    return true;
                }
                
                void* LockResource(const std::string& name) {
                    if (resources.find(name) != resources.end()) {
                        resources[name].isLocked = true;
                        return resources[name].data;
                    }
                    return nullptr;
                }
                
                void UnlockResource(const std::string& name) {
                    if (resources.find(name) != resources.end()) {
                        resources[name].isLocked = false;
                    }
                }
            };

            class EnhancedMemoryManager {
            private:
                PageAllocator pageAllocator;
                GraphicsResourceManager gfxResources;
                std::unordered_map memoryMap;
                VideoMemoryPool videoPool;
                SwapManager swapManager;
                MemoryOptimizer optimizer;
                size_t gameMemoryReserved;
                bool cachingEnabled;
                
                struct MemoryStats {
                    size_t totalAllocated;
                    size_t peakUsage;
                    size_t fragmentationCount;
                } stats;

            public:
                EnhancedMemoryManager() : stats{0, 0, 0} {
                    gameMemoryReserved = 4096; // 4GB in MB
                    cachingEnabled = true;
                    initMemory();
                }
                
                void* AllocateGameMemory(size_t size) {
                    void* ptr = pageAllocator.AllocateAligned(size, 4096);
                    if (ptr) {
                        memoryMap[ptr] = size;
                        stats.totalAllocated += size;
                        stats.peakUsage = std::max(stats.peakUsage, stats.totalAllocated);
                    }
                    return ptr;
                }
                
                bool AllocateGraphicsResource(const std::string& name, size_t size) {
                    return gfxResources.AllocateResource(name, size, "graphics");
                }
                
                void* LockGraphicsResource(const std::string& name) {
                    return gfxResources.LockResource(name);
                }
                
                void UnlockGraphicsResource(const std::string& name) {
                    gfxResources.UnlockResource(name);
                }
                
                void FreeGameMemory(void* ptr) {
                    if (memoryMap.find(ptr) != memoryMap.end()) {
                        stats.totalAllocated -= memoryMap[ptr];
                        memoryMap.erase(ptr);
                        pageAllocator.Deallocate(ptr);
                    }
                }
                
                void initializeVideoMemory(size_t poolSize = 8ULL * 1024 * 1024 * 1024) {
                    videoPool.initialize(poolSize);
                    reserveGameMemory();
                }

                void configureSwapSpace(size_t size = 16ULL * 1024 * 1024 * 1024) {
                    swapManager.configure(size);
                    enableCaching();
                }

                void optimizeForGaming() {
                    optimizer.optimize();
                    reserveGameMemory();
                    enableCaching();
                }
                
                MemoryStats GetStats() const {
                    return stats;
                }
                
                void DefragmentMemory() {
                    stats.fragmentationCount = 0;
                }

            private:
                void initMemory() {
                    reserveGameMemory();
                    enableCaching();
                }

                void reserveGameMemory() {
                    videoPool.reserve(gameMemoryReserved);
                }

                void enableCaching() {
                    if(cachingEnabled) {
                        optimizer.enableAggressiveCaching();
                    }
                }
            };
        