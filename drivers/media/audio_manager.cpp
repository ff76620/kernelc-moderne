
#pragma once
#include "../include/types.hpp"

namespace Media {

enum class AudioFormat {
    PCM_8,
    PCM_16,
    PCM_24,
    PCM_32,
    FLOAT_32,
    FLOAT_64,
    MP3,
    AAC,
    FLAC,
    OPUS,
    OGG,
    WAV,
    WMA,
    M3U
};

struct AudioDevice {
    char name[64];
    bool isOutput;
    bool isInput;
    uint32_t sampleRate;
    uint8_t channels;
    AudioFormat format;
    void* driverData;
};

class AudioManager {
private:
    static AudioDevice* devices;
    static size_t deviceCount;
    static Spinlock lock;

    struct AudioBuffer {
        void* data;
        size_t size;
        size_t position;
        bool loop;
    };

public:
    Status initialize();
    
    // Device management
    Status enumerateDevices();
    Status registerDevice(AudioDevice* device);
    Status unregisterDevice(AudioDevice* device);
    
    // Playback control
    Status play(AudioDevice* device, const void* data, size_t size, AudioFormat format);
    Status pause(AudioDevice* device);
    Status resume(AudioDevice* device);
    Status stop(AudioDevice* device);
    
    // Volume control
    Status setVolume(AudioDevice* device, float volume);
    Status getVolume(AudioDevice* device, float* volume);
    Status setMute(AudioDevice* device, bool mute);
    
    // Recording
    Status startRecording(AudioDevice* device, AudioFormat format);
    Status stopRecording(AudioDevice* device);
    Status getRecordedData(AudioDevice* device, void* buffer, size_t* size);
    
    // Format support
    bool isFormatSupported(AudioDevice* device, AudioFormat format);
    Status getFormatInfo(AudioFormat format, void* info, size_t size);
};

} // namespace Media