#ifndef STREAM_MANAGER_HPP
#define STREAM_MANAGER_HPP

#include 
#include 
#include 
#include 
#include 
#include "../include/types.hpp"

namespace Kernel {
namespace Multimedia {

class StreamManager {
public:
    enum class StreamType {
        Audio,
        Video,
        Subtitle,
        Data
    };

    struct StreamConfig {
        StreamType type;
        uint32_t bitrate;
        uint32_t bufferSize;
        bool realtime;
        std::string codec;
        std::map metadata;
    };

    struct StreamStats {
        uint64_t bytesProcessed;
        double avgBitrate;
        uint32_t bufferHealth;
        uint32_t dropCount;
        double latency;
    };

    StreamManager();
    ~StreamManager();

    bool initialize();
    void shutdown();

    // Stream control
    bool createStream(const std::string& streamId, const StreamConfig& config);
    bool destroyStream(const std::string& streamId);
    bool pauseStream(const std::string& streamId);
    bool resumeStream(const std::string& streamId);

    // Data handling
    bool pushData(const std::string& streamId, const std::vector& data);
    bool pullData(const std::string& streamId, std::vector& data);
    
    // Buffer management
    void setBufferSize(const std::string& streamId, uint32_t size);
    void setLatencyTarget(const std::string& streamId, double ms);
    bool flush(const std::string& streamId);
    
    // Synchronization
    bool sync(const std::string& masterStream, const std::string& slaveStream);
    void setSyncThreshold(double ms);
    
    // Monitoring
    StreamStats getStreamStats(const std::string& streamId) const;
    bool isStreamActive(const std::string& streamId) const;
    uint32_t getActiveStreamCount() const;

    // Quality control
    void setQualityProfile(const std::string& streamId, uint32_t profile);
    void enableAdaptiveBitrate(const std::string& streamId, bool enable);
    
    // Error handling
    struct StreamError {
        std::string message;
        uint32_t code;
        std::string details;
    };
    
    StreamError getLastError() const;

private:
    struct StreamContext {
        StreamConfig config;
        StreamStats stats;
        std::queue> buffer;
        bool active;
        std::thread processor;
        std::mutex mutex;
        std::condition_variable condition;
    };

    std::map> streams;
    std::mutex streamsMutex;
    StreamError lastError;
    
    // Internal methods
    void processStream(const std::string& streamId);
    bool validateStreamConfig(const StreamConfig& config);
    void updateStreamStats(StreamContext& context);
    void handleBufferOverflow(StreamContext& context);
    void adaptBitrate(StreamContext& context);
    
    // Resource management
    void cleanupResources();
    void optimizeMemoryUsage();
};

} // namespace Multimedia
} // namespace Kernel

#endif