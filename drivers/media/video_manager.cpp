
#pragma once
#include "../include/types.hpp"

namespace Media {

enum class VideoFormat {
    RGB888,
    RGBA8888,
    YUV420,
    YUV422,
    YUV444,
    H264,
    H265,
    VP8,
    VP9,
    AV1,
    MP4,
    MKV,
    VP6,
    MOV,
    WMV,
    AVI
};

struct VideoDevice {
    char name[64];
    uint32_t maxWidth;
    uint32_t maxHeight;
    uint32_t frameRate;
    bool supportsEncoding;
    bool supportsDecoding;
    void* driverData;
};

class VideoManager {
private:
    static VideoDevice* devices;
    static size_t deviceCount;
    static Spinlock lock;

    struct VideoBuffer {
        void* data;
        size_t size;
        uint32_t width;
        uint32_t height;
        VideoFormat format;
    };

public:
    Status initialize();
    
    // Device management
    Status enumerateDevices();
    Status registerDevice(VideoDevice* device);
    Status unregisterDevice(VideoDevice* device);
    
    // Video operations
    Status startStream(VideoDevice* device, uint32_t width, uint32_t height, VideoFormat format);
    Status stopStream(VideoDevice* device);
    Status getFrame(VideoDevice* device, VideoBuffer* buffer);
    
    // Hardware acceleration
    Status initializeHardwareAcceleration();
    Status encodeFrame(VideoDevice* device, const VideoBuffer* input, VideoBuffer* output, VideoFormat targetFormat);
    Status decodeFrame(VideoDevice* device, const VideoBuffer* input, VideoBuffer* output, VideoFormat targetFormat);
    
    // Format support
    bool isFormatSupported(VideoDevice* device, VideoFormat format);
    Status getFormatInfo(VideoFormat format, void* info, size_t size);
    
    // Device properties
    Status getDeviceCapabilities(VideoDevice* device, void* caps, size_t size);
    Status setDeviceProperty(VideoDevice* device, const char* property, const void* value);
};

} // namespace Media