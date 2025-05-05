
#pragma once
#include "../include/types.hpp"

namespace Multimedia {

enum class VideoFormat {
    RGB888,
    RGBA8888,
    YUV420,
    YUV422,
    H264,
    H265,
    VP8,
    VP9,
    AV1,
    MP4,
    MKV,
    VP6,
    MOV,     // Format ajouté
    WMV,     // Format ajouté  
    AVI      // Format ajouté
};

struct VideoDevice {
    char name[64];
    uint32_t maxResolutionX;
    uint32_t maxResolutionY;
    uint32_t frameRates[8];
    uint8_t frameRateCount;
    VideoFormat supportedFormats;
    void* driverData;
};

class VideoManager {
private:
    struct VideoStream {
        VideoFormat format;
        uint32_t width;
        uint32_t height;
        uint32_t frameRate;
        void* buffer;
        size_t bufferSize;
        void* containerContext;
    };

    VideoDevice* devices;
    size_t deviceCount;
    Spinlock lock;

public:
    Status initialize();
    Status enumerateDevices();
    
    // Video device operations
    Status openDevice(const char* name, VideoDevice** device);
    Status closeDevice(VideoDevice* device);
    Status setFormat(VideoDevice* device, VideoFormat format);
    
    // Video streaming
    Status createStream(VideoDevice* device, VideoStream** stream);
    Status writeFrame(VideoStream* stream, const void* data, size_t size);
    Status readFrame(VideoStream* stream, void* data, size_t size);
    
    // Video processing
    Status scaleFrame(VideoStream* stream, uint32_t newWidth, uint32_t newHeight);
    Status convertFormat(VideoStream* stream, VideoFormat newFormat);
    Status applyFilter(VideoStream* stream, uint32_t filterType, const void* params);
    
    // Formats spécifiques
    Status handleMP4Container(VideoStream* stream);
    Status handleMKVContainer(VideoStream* stream);
    Status handleMOVContainer(VideoStream* stream); // Nouveau
    Status handleWMVContainer(VideoStream* stream); // Nouveau
    Status handleAVIContainer(VideoStream* stream); // Nouveau
    Status decodeVP6(VideoStream* stream);
    Status extractSubtitles(VideoStream* stream, void* subtitleData);
    Status extractAudio(VideoStream* stream, AudioStream** audioStream);
};

} // namespace Multimedia