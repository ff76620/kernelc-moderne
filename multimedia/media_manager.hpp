#ifndef MEDIA_MANAGER_HPP
#define MEDIA_MANAGER_HPP

#include 
#include 
#include 
#include "../include/types.hpp"
#include "../multimedia/pipeline_manager.hpp"
#include "../multimedia/effects_manager.hpp"
#include "../multimedia/codec_manager.hpp"

namespace Kernel {
namespace Multimedia {

class MediaManager {
public:
    MediaManager();
    ~MediaManager();

    bool initialize();
    void shutdown();

    // Stream management
    StreamHandle createStream(MediaType type, uint32_t priority);
    bool destroyStream(StreamHandle handle);
    bool pauseStream(StreamHandle handle);
    bool resumeStream(StreamHandle handle);

    // Data handling
    bool pushData(StreamHandle handle, const void* data, size_t size);
    bool readData(StreamHandle handle, void* buffer, size_t size);
    
    // Configuration
    void setStreamQuality(StreamHandle handle, uint32_t quality);
    void setStreamBitrate(StreamHandle handle, uint32_t bitrate);
    void enableHardwareAcceleration(bool enable);

    // Performance monitoring
    struct StreamStats {
        double processedFrames;
        double frameRate;
        double bitrate;
        double latency;
        uint32_t droppedFrames;
        uint32_t bufferHealth;
    };

    StreamStats getStreamStats(StreamHandle handle) const;
    
private:
    std::unique_ptr pipelineManager;
    std::unique_ptr effectsManager;
    std::unique_ptr codecManager;
    
    std::map streams;
    bool hardwareAccelEnabled;

    void optimizePerformance();
    void handleErrors();
};

} // namespace Multimedia
} // namespace Kernel

#endif