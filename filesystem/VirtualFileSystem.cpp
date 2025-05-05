
#include "kernel/filesystem/VirtualFileSystem.hpp"
#include "kernel/filesystem/FileCache.hpp"
#include "kernel/include/types.hpp"
#include "kernel/loggin/EventLogger.hpp"
#include 
#include 
#include 
#include 
#include 
#include 
#include 

namespace Kernel {
namespace FileSystem {

// Compression Manager Implementation
class CompressionManager {
    static constexpr size_t COMPRESS_THRESHOLD = 4096;
    
    struct CompressionStats {
        std::atomic bytes_saved{0};
        std::atomic total_compressed{0};
    } stats;

public:
    std::vector compress(const std::vector& data) {
        if(data.size() < COMPRESS_THRESHOLD) return data;
        
        // Use zlib for compression
        z_stream strm;
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL; 
        strm.opaque = Z_NULL;
        
        if (deflateInit(&strm, Z_BEST_COMPRESSION) != Z_OK) {
            return data;
        }

        std::vector compressed(data.size());
        
        strm.avail_in = data.size();
        strm.next_in = (Bytef*)data.data();
        strm.avail_out = compressed.size();
        strm.next_out = (Bytef*)compressed.data();

        int ret = deflate(&strm, Z_FINISH);
        deflateEnd(&strm);

        if (ret != Z_STREAM_END) {
            return data;
        }

        compressed.resize(strm.total_out);
        
        if(compressed.size() < data.size()) {
            stats.bytes_saved += (data.size() - compressed.size());
            stats.total_compressed++;
            return compressed;
        }
        return data;
    }
    
    std::vector decompress(const std::vector& data) {
        z_stream strm;
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        
        if (inflateInit(&strm) != Z_OK) {
            return data;
        }

        std::vector decompressed(data.size() * 2); // Initial estimate
        
        strm.avail_in = data.size();
        strm.next_in = (Bytef*)data.data();
        strm.avail_out = decompressed.size();
        strm.next_out = (Bytef*)decompressed.data();

        while (true) {
            int ret = inflate(&strm, Z_NO_FLUSH);
            
            if (ret == Z_STREAM_END) {
                break;
            }
            
            if (ret != Z_OK) {
                inflateEnd(&strm);
                return data;
            }

            // Expand buffer if needed
            if (strm.avail_out == 0) {
                size_t currentSize = decompressed.size();
                decompressed.resize(currentSize * 2);
                strm.next_out = (Bytef*)(decompressed.data() + currentSize);
                strm.avail_out = currentSize;
            }
        }

        decompressed.resize(strm.total_out);
        inflateEnd(&strm);
        return decompressed;
    }
};

// File System Encryption
class EncryptionManager {
    std::vector key;
    std::mutex key_mutex;

public:
    bool initialize() {
        std::lock_guard lock(key_mutex);
        
        // Generate random 256-bit key using secure random number generator
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution dis;

        key.resize(32);
        for(int i = 0; i < 32; i++) {
            key[i] = dis(gen);
        }

        // Save key securely
        std::ofstream keyFile("keystore.bin", std::ios::binary);
        if(!keyFile) return false;

        // Add salt and iteration count for key derivation
        uint8_t salt[16];
        for(int i = 0; i < 16; i++) salt[i] = dis(gen);
        
        uint32_t iterations = 10000;
        
        keyFile.write((char*)salt, sizeof(salt));
        keyFile.write((char*)&iterations, sizeof(iterations));
        keyFile.write((char*)key.data(), key.size());

        return true;
    }
    
    std::vector encrypt(const std::vector& data) {
        std::lock_guard lock(key_mutex);
        
        // Generate random IV
        std::vector iv(16);
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution dis;
        for(int i = 0; i < 16; i++) {
            iv[i] = dis(gen);
        }

        // Initialize AES in CBC mode
        EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
        EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key.data(), iv.data());

        // Allocate encrypted buffer with space for padding
        std::vector encrypted(data.size() + EVP_MAX_BLOCK_LENGTH);
        int outlen1, outlen2;

        // Encrypt data
        EVP_EncryptUpdate(ctx, (unsigned char*)encrypted.data(), &outlen1,
                          (unsigned char*)data.data(), data.size());
        EVP_EncryptFinal_ex(ctx, (unsigned char*)(encrypted.data() + outlen1), &outlen2);

        EVP_CIPHER_CTX_free(ctx);

        // Resize to actual encrypted size
        encrypted.resize(outlen1 + outlen2);

        // Prepend IV to encrypted data
        encrypted.insert(encrypted.begin(), iv.begin(), iv.end());
        
        return encrypted;
    }
    
    std::vector decrypt(const std::vector& data) {
        std::lock_guard lock(key_mutex);

        // Extract IV from beginning of data
        std::vector iv(data.begin(), data.begin() + 16);
        std::vector encryptedData(data.begin() + 16, data.end());

        // Initialize AES in CBC mode
        EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
        EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key.data(), iv.data());

        // Allocate decrypted buffer
        std::vector decrypted(encryptedData.size());
        int outlen1, outlen2;

        // Decrypt data
        EVP_DecryptUpdate(ctx, (unsigned char*)decrypted.data(), &outlen1,
                          (unsigned char*)encryptedData.data(), encryptedData.size());
        EVP_DecryptFinal_ex(ctx, (unsigned char*)(decrypted.data() + outlen1), &outlen2);

        EVP_CIPHER_CTX_free(ctx);

        // Resize to actual decrypted size
        decrypted.resize(outlen1 + outlen2);
        
        return decrypted;
    }
};

// Advanced Error Handling
class ErrorManager {
    struct ErrorContext {
        std::string operation;
        std::string path;
        int error_code;
        std::chrono::system_clock::time_point timestamp;
    };
    
    std::deque error_log;
    std::mutex log_mutex;
    
public:
    void logError(const std::string& op, const std::string& path, int code) {
        std::lock_guard lock(log_mutex);
        error_log.push_back({op, path, code, std::chrono::system_clock::now()});
        
        if(error_log.size() > 1000) { // Keep last 1000 errors
            error_log.pop_front();
        }
    }
    
    std::vector getRecentErrors(size_t count = 10) {
        std::lock_guard lock(log_mutex);
        std::vector recent;
        auto start = error_log.rbegin();
        auto end = error_log.rbegin() + std::min(count, error_log.size());
        std::copy(start, end, std::back_inserter(recent));
        return recent;
    }
};

// Buffer Pool Implementation
class BufferPool {
    static constexpr size_t BUFFER_SIZE = 4096;
    static constexpr size_t POOL_SIZE = 1024;
    
    struct Buffer {
        std::array data;
        bool in_use = false;
    };
    
    std::vector pool;
    std::mutex pool_mutex;

public:
    BufferPool() : pool(POOL_SIZE) {}
    
    Buffer* acquire() {
        std::lock_guard lock(pool_mutex);
        for(auto& buffer : pool) {
            if(!buffer.in_use) {
                buffer.in_use = true;
                return &buffer;
            }
        }
        return nullptr;
    }
    
    void release(Buffer* buffer) {
        std::lock_guard lock(pool_mutex);
        buffer->in_use = false;
    }
    
    void expand(size_t size) {
        std::lock_guard lock(pool_mutex);
        pool.resize(pool.size() + size);
    }
};

// Multi-level Cache Implementation
class MultiLevelCache {
    struct CacheEntry {
        std::vector data;
        std::chrono::steady_clock::time_point timestamp;
        bool dirty = false;
    };

    struct CacheLevel {
        size_t size;
        double hit_ratio;
        std::unordered_map data;
        std::atomic hit_count{0};
        std::atomic miss_count{0};
    };

    struct CacheMetrics {
        size_t levelIndex;
        size_t usedSize;
        size_t totalSize; 
        double hitRatio;
    };

    struct AccessPattern {
        std::chrono::steady_clock::time_point lastAccess;
        size_t accessCount = 0;
        size_t levelIndex;
        size_t size;
    };

    std::vector levels;
    std::shared_mutex cache_mutex;

public:
    MultiLevelCache() {
        // Initialize cache levels (L1: 1MB, L2: 8MB, L3: 32MB)
        addLevel(1 * 1024 * 1024);
        addLevel(8 * 1024 * 1024);
        addLevel(32 * 1024 * 1024);
    }

    void addLevel(size_t size) {
        levels.push_back({size, 0.0, {}});
    }
    
    bool get(const std::string& key, std::vector& data) {
        std::shared_lock lock(cache_mutex);
        for(size_t i = 0; i < levels.size(); i++) {
            auto& level = levels[i];
            auto it = level.data.find(key);
            if(it != level.data.end()) {
                data = it->second.data;
                it->second.timestamp = std::chrono::steady_clock::now();
                level.hit_count++;
                promoteToHigherLevel(key, data, i);
                return true;
            }
            level.miss_count++;
        }
        return false;
    }
    
    void put(const std::string& key, const std::vector& data, bool dirty) {
        std::lock_guard lock(cache_mutex);
        for(size_t i = 0; i < levels.size(); i++) {
            auto& level = levels[i];
            auto it = level.data.find(key);
            if(it != level.data.end()) {
                it->second.data = data;
                it->second.timestamp = std::chrono::steady_clock::now();
                it->second.dirty = dirty;
                return;
            }
        }
        levels[0].data[key] = {data, std::chrono::steady_clock::now(), dirty};
    }
    
    void expandLevel(size_t level, size_t size) {
        std::lock_guard lock(cache_mutex);
        levels[level].size += size;
    }
    
    void optimizeCache() {
        std::lock_guard lock(cache_mutex);
        
        // Calculate current cache usage and hit rates for each level
        std::vector levelMetrics;
        for(size_t i = 0; i < levels.size(); i++) {
            auto& level = levels[i];
            
            size_t currentSize = 0;
            for(const auto& [key, entry] : level.data) {
                currentSize += entry.data.size();
            }
            
            CacheMetrics metrics;
            metrics.levelIndex = i;
            metrics.usedSize = currentSize;
            metrics.totalSize = level.size;
            metrics.hitRatio = level.hit_ratio;
            levelMetrics.push_back(metrics);
        }

        // Analyze access patterns
        auto now = std::chrono::steady_clock::now();
        std::map accessPatterns;
        
        for(size_t i = 0; i < levels.size(); i++) {
            for(const auto& [key, entry] : levels[i].data) {
                auto timeSinceAccess = std::chrono::duration_cast(
                    now - entry.timestamp).count();
                    
                // Track access frequency and recency
                accessPatterns[key].lastAccess = entry.timestamp;
                accessPatterns[key].accessCount++;
                accessPatterns[key].levelIndex = i;
                accessPatterns[key].size = entry.data.size();
            }
        }

        // Optimize cache levels based on metrics
        for(size_t i = 0; i < levels.size(); i++) {
            auto& level = levels[i];
            
            // If level is too full (>90%), evict least recently used entries
            if(levelMetrics[i].usedSize > levelMetrics[i].totalSize * 0.9) {
                std::vector> entries;
                for(const auto& [key, entry] : level.data) {
                    entries.emplace_back(key, entry.timestamp);
                }
                
                // Sort by access time
                std::sort(entries.begin(), entries.end(),
                    [](const auto& a, const auto& b) {
                        return a.second < b.second;
                    });
                    
                // Remove oldest entries until usage is below 70%
                size_t removedSize = 0;
                for(const auto& entry : entries) {
                    if(levelMetrics[i].usedSize - removedSize < levelMetrics[i].totalSize * 0.7) {
                        break;
                    }
                    removedSize += level.data[entry.first].data.size();
                    level.data.erase(entry.first);
                }
            }
            
            // Move frequently accessed entries to higher cache level
            if(i > 0) {
                for(const auto& [key, entry] : level.data) {
                    auto& pattern = accessPatterns[key];
                    if(pattern.accessCount > 100 && // High access count
                       std::chrono::duration_cast(
                           now - pattern.lastAccess).count() < 60) { // Recent access
                        
                        // Move to higher cache level if space permits
                        if(levelMetrics[i-1].usedSize + entry.data.size() <= levelMetrics[i-1].totalSize) {
                            levels[i-1].data[key] = entry;
                            level.data.erase(key);
                        }
                    }
                }
            }
        }

        // Update hit ratios
        for(size_t i = 0; i < levels.size(); i++) {
            uint64_t hits = levels[i].hit_count;
            uint64_t misses = levels[i].miss_count;
            if(hits + misses > 0) {
                levels[i].hit_ratio = static_cast(hits) / (hits + misses);
            }
            // Reset counters
            levels[i].hit_count = 0;
            levels[i].miss_count = 0;
        }

        // Log optimization results
        std::stringstream log;
        log << "Cache optimization completed:\n";
        for(size_t i = 0; i < levels.size(); i++) {
            log << "Level " << i << ": "
                << "Size: " << levelMetrics[i].usedSize << "/" << levelMetrics[i].totalSize
                << " Hit ratio: " << levels[i].hit_ratio << "\n";
        }
        EventLogger::log(log.str());
    }
    
    std::vector getAllEntries() {
        std::lock_guard lock(cache_mutex);
        std::vector entries;
        for(const auto& level : levels) {
            for(const auto& pair : level.data) {
                entries.push_back(pair.second);
            }
        }
        return entries;
    }
};

// Performance Metrics Implementation
class PerformanceMetrics {
    struct Metrics {
        std::atomic read_ops{0};
        std::atomic write_ops{0};
        std::atomic cache_hits{0};
        std::atomic cache_misses{0};
        std::atomic total_bytes_read{0};
        std::atomic total_bytes_written{0};
        std::atomic total_bytes_compressed{0};
        std::chrono::steady_clock::time_point start_time;
        std::vector read_latencies;
    } metrics;

public:
    void recordRead(size_t bytes) {
        metrics.read_ops++;
        metrics.total_bytes_read += bytes;
    }
    
    void recordWrite(size_t bytes) {
        metrics.write_ops++;
        metrics.total_bytes_written += bytes;
    }
    
    void recordCacheHit() { metrics.cache_hits++; }
    void recordCacheMiss() { metrics.cache_misses++; }
    
    void recordCompression(size_t bytes) {
        metrics.total_bytes_compressed += bytes;
    }
    
    void recordReadLatency(std::chrono::microseconds latency) {
        metrics.read_latencies.push_back(latency);
    }
    
    double getCacheHitRatio() {
        uint64_t total = metrics.cache_hits + metrics.cache_misses;
        return total ? (double)metrics.cache_hits / total : 0.0;
    }
    
    double getCompressionRatio() {
        uint64_t total = metrics.total_bytes_read + metrics.total_bytes_written;
        return total ? (double)metrics.total_bytes_compressed / total : 0.0;
    }
    
    double getAverageReadLatency() {
        if(metrics.read_latencies.empty()) return 0.0;
        double sum = 0.0;
        for(const auto& latency : metrics.read_latencies) {
            sum += latency.count();
        }
        return sum / metrics.read_latencies.size();
    }
    
    double getThroughput() {
        auto elapsed = std::chrono::steady_clock::now() - metrics.start_time;
        return (double)(metrics.total_bytes_read + metrics.total_bytes_written) / 
               std::chrono::duration_cast(elapsed).count() / 
               (1024 * 1024); // Convert to MB/s
    }
};

// Memory Mapped File Implementation
class MemoryMappedFile {
    void* mapped_region = nullptr;
    size_t file_size = 0;
    std::string filename;
    
public:
    bool map(const std::string& path) {
        #ifdef _WIN32
            HANDLE file = CreateFileA(path.c_str(), GENERIC_READ | GENERIC_WRITE,
                                    FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL, nullptr);
            if(file == INVALID_HANDLE_VALUE) return false;
            
            HANDLE mapping = CreateFileMapping(file, nullptr, PAGE_READWRITE,
                                            0, 0, nullptr);
            if(!mapping) {
                CloseHandle(file);
                return false;
            }
            
            mapped_region = MapViewOfFile(mapping, FILE_MAP_ALL_ACCESS,
                                        0, 0, 0);
            CloseHandle(mapping);
            CloseHandle(file);
        #else
            int fd = open(path.c_str(), O_RDWR);
            if(fd == -1) return false;
            
            struct stat sb;
            if(fstat(fd, &sb) == -1) {
                close(fd);
                return false;
            }
            
            file_size = sb.st_size;
            mapped_region = mmap(nullptr, file_size, PROT_READ | PROT_WRITE,
                               MAP_SHARED, fd, 0);
            close(fd);
            
            if(mapped_region == MAP_FAILED) {
                mapped_region = nullptr;
                return false;
            }
        #endif
        
        filename = path;
        return mapped_region != nullptr;
    }
    
    void unmap() {
        if(!mapped_region) return;
        
        #ifdef _WIN32
            UnmapViewOfFile(mapped_region);
        #else
            munmap(mapped_region, file_size);
        #endif
        
        mapped_region = nullptr;
    }
    
    ~MemoryMappedFile() {
        unmap();
    }
    
    char* getData() {
        return static_cast(mapped_region);
    }
    
    size_t getSize() {
        return file_size;
    }
};

// Enhanced VirtualFileSystem Implementation
class VirtualFileSystem {
private:
    std::unique_ptr bufferPool;
    std::unique_ptr multiLevelCache;
    std::unique_ptr metrics;
    std::unique_ptr compression;
    std::unique_ptr encryption;
    std::unique_ptr errorManager;
    std::unordered_map> mappedFiles;
    std::mutex mutex;
    
    // Background Tasks
    std::atomic running{true};
    std::thread cacheManager;
    std::thread compressionWorker;
    std::thread metricCollector;
    const double TARGET_THROUGHPUT = 100.0; // MB/s
    
public:
    VirtualFileSystem() {
        bufferPool = std::make_unique();
        multiLevelCache = std::make_unique();
        metrics = std::make_unique();
        compression = std::make_unique();
        encryption = std::make_unique();
        errorManager = std::make_unique();
        
        startBackgroundTasks();
        metrics->metrics.start_time = std::chrono::steady_clock::now();
    }
    
    ~VirtualFileSystem() {
        running = false;
        if(cacheManager.joinable()) cacheManager.join();
        if(compressionWorker.joinable()) compressionWorker.join();
        if(metricCollector.joinable()) metricCollector.join();
    }
    
private:
    void startBackgroundTasks() {
        cacheManager = std::thread([this]() {
            while(running) {
                multiLevelCache->optimizeCache();
                std::this_thread::sleep_for(std::chrono::seconds(60));
            }
        });
        
        compressionWorker = std::thread([this]() {
            while(running) {
                compressInactiveFiles();
                std::this_thread::sleep_for(std::chrono::minutes(5));
            }
        });
        
        metricCollector = std::thread([this]() {
            while(running) {
                collectAndAnalyzeMetrics();
                std::this_thread::sleep_for(std::chrono::seconds(30));
            }
        });
    }
    
    void compressInactiveFiles() {
        std::lock_guard lock(mutex);
        
        // Get list of files that haven't been accessed in last hour
        auto now = std::chrono::steady_clock::now();
        std::vector inactiveFiles;
        
        for(const auto& [path, entry] : multiLevelCache->getAllEntries()) {
            auto timeSinceAccess = std::chrono::duration_cast(
                now - entry.timestamp).count();
                
            if(timeSinceAccess > 60) { // More than 1 hour
                inactiveFiles.push_back(path);
            }
        }
        
        // Compress each inactive file
        for(const auto& path : inactiveFiles) {
            std::vector data;
            if(multiLevelCache->get(path, data)) {
                auto compressed = compression->compress(data);
                if(compressed.size() < data.size()) {
                    // Update cache with compressed data
                    multiLevelCache->put(path, compressed, true);
                    metrics->recordCompression(data.size() - compressed.size());
                }
            }
        }
        
        EventLogger::log("Compressed " + std::to_string(inactiveFiles.size()) + 
                        " inactive files");
    }
    
    void collectAndAnalyzeMetrics() {
        std::lock_guard lock(mutex);
        
        // Gather current metrics
        auto cacheHitRatio = metrics->getCacheHitRatio();
        auto compressionRatio = metrics->getCompressionRatio();
        auto averageLatency = metrics->getAverageReadLatency();
        auto throughput = metrics->getThroughput();
        
        // Log metrics
        std::stringstream ss;
        ss << "Performance Metrics:\n"
           << "Cache Hit Ratio: " << std::fixed << std::setprecision(2) 
           << (cacheHitRatio * 100) << "%\n"
           << "Compression Ratio: " << std::fixed << std::setprecision(2)
           << (compressionRatio * 100) << "%\n" 
           << "Average Latency: " << averageLatency << "ms\n"
           << "Throughput: " << throughput << " MB/s";
           
        EventLogger::log(ss.str());
        
        // Optimize based on metrics
        if(cacheHitRatio < 0.5) {
            // Increase cache size
            multiLevelCache->expandLevel(0, 1024 * 1024); // Add 1MB to L1
        }
        
        if(throughput < TARGET_THROUGHPUT) {
            // Increase buffer pool size
            bufferPool->expand(64); // Add 64 buffers
        }
    }
    
    ssize_t read(FileDescriptor* fd, void* buffer, size_t size) {
        if (!fd || !buffer || size == 0) return -1;
        
        auto start = std::chrono::steady_clock::now();
        
        // Try to read from cache first
        std::vector cachedData;
        if (multiLevelCache->get(fd->path, cachedData)) {
            metrics->recordCacheHit();
            size_t copySize = std::min(size, cachedData.size() - fd->position);
            memcpy(buffer, cachedData.data() + fd->position, copySize);
            fd->position += copySize;
            metrics->recordRead(copySize);
            
            auto end = std::chrono::steady_clock::now();
            metrics->recordReadLatency(std::chrono::duration_cast(
                end - start).count());
                
            return copySize;
        }
        
        metrics->recordCacheMiss();
        
        // For large files, use memory mapping
        if (getFileSize(fd->path) > 1024 * 1024) { // 1MB threshold
            auto& mappedFile = mappedFiles[fd->path];
            if (!mappedFile) {
                mappedFile = std::make_unique();
                if (!mappedFile->map(fd->path)) {
                    EventLogger::log("Failed to memory map file: " + fd->path);
                    return -1;
                }
            }
            
            // Read from mapped memory
            char* mappedData = mappedFile->getData();
            size_t mappedSize = mappedFile->getSize();
            
            if(fd->position >= mappedSize) {
                return 0; // EOF
            }
            
            size_t copySize = std::min(size, mappedSize - fd->position);
            memcpy(buffer, mappedData + fd->position, copySize);
            fd->position += copySize;
            
            // Update cache
            std::vector data(mappedData + fd->position - copySize, 
                                 mappedData + fd->position);
            multiLevelCache->put(fd->path, data);
            
            auto end = std::chrono::steady_clock::now();
            metrics->recordReadLatency(std::chrono::duration_cast(
                end - start).count());
                
            return copySize;
        }
        
        // Use buffer pool for regular reads
        auto* bufferPoolBuffer = bufferPool->acquire();
        if (!bufferPoolBuffer) {
            EventLogger::log("Failed to acquire buffer from pool");
            return -1;
        }
        
        // Perform read operation using buffer
        std::ifstream file(fd->path, std::ios::binary);
        if (!file) {
            bufferPool->release(bufferPoolBuffer);
            EventLogger::log("Failed to open file: " + fd->path);
            return -1;
        }
        
        file.seekg(fd->position);
        file.read(bufferPoolBuffer->data.data(), bufferPoolBuffer->data.size());
        
        ssize_t bytesRead = file.gcount();
        if(bytesRead > 0) {
            size_t copySize = std::min(static_cast(bytesRead), size);
            memcpy(buffer, bufferPoolBuffer->data.data(), copySize);
            fd->position += copySize;
            
            // Update cache
            std::vector data(bufferPoolBuffer->data.data(),
                                 bufferPoolBuffer->data.data() + copySize);
            multiLevelCache->put(fd->path, data);
            
            bytesRead = copySize;
        }
        
        bufferPool->release(bufferPoolBuffer);
        
        auto end = std::chrono::steady_clock::now();
        metrics->recordReadLatency(std::chrono::duration_cast(
            end - start).count());
            
        return bytesRead;
    }
    
    size_t getFileSize(const std::string& path) {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file) {
            return 0;
        }
        return file.tellg();
    }
};

} // namespace FileSystem
} // namespace Kernel