#pragma once
#include "../include/types.hpp"

namespace Multimedia {

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
    OGG,
    WAV,
    WMA,
    M3U
};

struct AudioDevice {
    char name[64];
    uint32_t sampleRates[8];
    uint8_t sampleRateCount;
    AudioFormat supportedFormats;
    uint8_t channels;
    void* driverData;
};

class AudioManager {
private:
    struct AudioStream {
        AudioFormat format;
        uint32_t sampleRate;
        uint8_t channels;
        void* buffer;
        size_t bufferSize;
        bool isPlaylist;  // Pour les formats M3U
    };

    AudioDevice* devices;
    size_t deviceCount;
    Spinlock lock;

public:
    Status initialize();
    Status enumerateDevices();
    
    // Audio device operations
    Status openDevice(const char* name, AudioDevice** device);
    Status closeDevice(AudioDevice* device);
    Status setFormat(AudioDevice* device, AudioFormat format);
    
    // Audio streaming
    Status createStream(AudioDevice* device, AudioStream** stream);
    Status writeStream(AudioStream* stream, const void* data, size_t size);
    Status readStream(AudioStream* stream, void* data, size_t size);
    
    // Audio processing
    Status mixStreams(AudioStream** streams, size_t count, AudioStream* output);
    Status applyEffect(AudioStream* stream, uint32_t effectType, const void* params);
    
    // Formats spécifiques
    Status parsePlaylist(const char* path, AudioStream** streams, size_t* count);
    Status convertToWAV(AudioStream* stream);
    Status handleWMAStream(AudioStream* stream);
};

} // namespace Multimedia