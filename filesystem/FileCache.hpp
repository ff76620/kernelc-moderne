#ifndef UNIVERSAL_FILE_CACHE_HPP
#define UNIVERSAL_FILE_CACHE_HPP

#include <array>
#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include <thread>

namespace Kernel {

// Supported filesystem formats with comprehensive format support
enum class FileSystemFormat {
  NTFS, FAT32, EXT4, EXFAT, APFS, BTRFS, HFS_PLUS, XFS, ZFS, UFS,
  REFS, F2FS, NILFS, JFS, MINIX, HAMMER, AFS, GFS2, OCFS2, RAW
};

// Enhanced cache entry with format-specific optimizations
struct CacheEntry {
  alignas(64) std::vector<uint8_t> data; // Cache line aligned
  std::chrono::steady_clock::time_point lastAccess;
  std::atomic<uint64_t> hitCount{0};
  std::atomic<bool> dirty{false}; 
  std::atomic<bool> locked{false};
  uint32_t priority{0};
  FileSystemFormat format;
  
  // Advanced functionality
  struct CompressionStats {
    size_t originalSize;
    size_t compressedSize;
    double ratio;
    std::chrono::nanoseconds compressionTime;
    bool hardwareAccelerated;
  } compressionStats;

  struct DMARegion {
    void* physAddr;
    size_t size;
    bool pinned;
    uint32_t alignmentMask;
  } dmaRegion;

  std::unique_ptr<FormatSpecificPrefetcher> prefetcher;
  
  // Cache coherency
  std::shared_mutex coherencyMutex;
  std::vector<CacheLine> cacheLines;
  bool isCoherent{true};

  // Hardware acceleration
  GPUBuffer gpuBuffer;
  bool gpuResident{false};
};

class UniversalFileCache {
private:
  // Multi-level hybrid cache with intelligent sizing
  struct CacheLevel {
    LRUCache<CacheEntry> lruCache;
    ARCache<CacheEntry> arcCache; 
    PriorityQueue<CacheEntry> priorityCache;
    size_t size;
    size_t maxSize;
    std::atomic<double> hitRate{0.0};
    std::atomic<uint64_t> averageLatency{0};
  };
  
  std::array<CacheLevel, 3> cacheLevels;
  
  // Format-specific handlers with specialized optimizations
  std::unordered_map<FileSystemFormat, std::unique_ptr<FormatHandler>> formatHandlers;
  
  // Advanced I/O subsystem
  DMAEngine dmaEngine;
  WriteCoalescer writeCoalescer;
  PrefetchPredictor prefetchPredictor;
  
  // Hardware acceleration
  GPUCompressor gpuCompressor;
  HardwareHasher hwHasher;
  
  // Performance monitoring
  std::atomic<size_t> hits{0};
  std::atomic<size_t> misses{0};
  std::atomic<uint64_t> totalLatency{0};
  std::atomic<uint64_t> throughput{0};
  PerformanceMonitor perfMonitor;
  
  // Configuration
  const size_t MIN_CACHE_SIZE = 1024 * 1024; // 1MB 
  const size_t MAX_CACHE_SIZE = 549755813888ULL; // 512GB
  std::atomic<size_t> currentSize{0};
  
public:
  UniversalFileCache() {
    initializeCacheLevels();
    initializeFormatHandlers();
    configureDMA();
    initializeHardwareAcceleration();
    startBackgroundTasks();
  }

  template<typename T>
  Result<T> read(const std::string& path, size_t offset, size_t size) {
    auto start = std::chrono::steady_clock::now();
    
    // Try hardware-accelerated hash lookup first
    auto hash = hwHasher.computeHash(path);
    
    // Check each cache level with SIMD optimization
    for(auto& level : cacheLevels) {
      if(auto entry = level.lruCache.get(hash)) {
        updateStats(true, start);
        return retrieveData<T>(*entry, offset, size);
      }
      
      if(auto entry = level.arcCache.get(hash)) {
        updateStats(true, start);
        return retrieveData<T>(*entry, offset, size);
      }
    }
    
    // Cache miss - load from disk with format-specific optimizations
    auto format = detectFormat(path);
    auto handler = formatHandlers[format].get();
    
    auto data = handler->read(path, offset, size);
    if(!data) {
      updateStats(false, start);
      return Error("Read failed");
    }
    
    // Optimize and cache the data
    auto entry = createOptimizedCacheEntry(std::move(*data), format);
    insertIntoCache(hash, std::move(entry));
    
    updateStats(false, start);
    return retrieveData<T>(*entry, offset, size);
  }

  template<typename T>
  bool write(const std::string& path, const T& data, size_t offset) {
    // Queue write with compression if beneficial
    if(shouldCompress(data)) {
      auto compressed = gpuCompressor.compress(data);
      writeCoalescer.queue(path, compressed, offset);
    } else {
      writeCoalescer.queue(path, data, offset);
    }
    
    // Intelligent flush decision
    if(writeCoalescer.shouldFlush()) {
      return flushWrites();
    }
    return true;
  }

private:
  void initializeCacheLevels() {
    // L1 - Ultra-fast cache (20%)
    cacheLevels[0].maxSize = MAX_CACHE_SIZE * 0.2;
    
    // L2 - Balanced cache (30%)
    cacheLevels[1].maxSize = MAX_CACHE_SIZE * 0.3;
    
    // L3 - High-capacity cache (50%)
    cacheLevels[2].maxSize = MAX_CACHE_SIZE * 0.5;
    
    for(auto& level : cacheLevels) {
      level.size = 0;
    }
  }

  void initializeFormatHandlers() {
    // Initialize handlers for all supported formats
    for(const auto& format : getAllFormats()) {
      formatHandlers[format] = createOptimizedHandler(format);
    }
  }

  void configureDMA() {
    dmaEngine.setMode(DMAMode::ZERO_COPY);
    dmaEngine.setPageSize(2_MB);
    dmaEngine.enableScatterGather();
    dmaEngine.setMaxTransferSize(128_MB);
    dmaEngine.setInterruptThreshold(16_KB);
    dmaEngine.enableHardwareCompression();
  }

  void startBackgroundTasks() {
    std::thread([this]() {
      while(true) {
        maintainCacheCoherency();
        optimizeCache();
        prefetchPredicted();
        updatePerformanceMetrics();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
      }
    }).detach();
  }

  bool flushWrites() {
    auto writes = writeCoalescer.getOptimizedWrites();
    
    for(const auto& write : writes) {
      auto format = detectFormat(write.path);
      auto handler = formatHandlers[format].get();
      
      if(!handler->write(write.path, write.data, write.offset)) {
        return false;
      }
      
      updateCacheEntry(write);
    }
    
    writeCoalescer.clear();
    return true;
  }

  void updateStats(bool hit, std::chrono::steady_clock::time_point start) {
    auto end = std::chrono::steady_clock::now();
    auto latency = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    
    if(hit) hits++; else misses++;
    totalLatency += latency;
    
    auto bytesProcessed = hit ? currentSize.load() : 0;
    throughput = bytesProcessed / (latency / 1e9);
    
    perfMonitor.recordMetrics(hit, latency, bytesProcessed);
  }
};

} // namespace Kernel

#endif