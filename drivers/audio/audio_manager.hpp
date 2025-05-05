
#pragma once
#include "audio_device.hpp"
#include 

namespace Audio {

class AudioManager {
private:
    std::vector devices;
    AudioDevice* defaultPlaybackDevice;
    AudioDevice* defaultCaptureDevice;
    float masterVolume;
    bool muted;

public:
    Status initialize();
    Status shutdown();
    
    // Device management
    Status enumerateDevices();
    Status addDevice(AudioDevice* device);
    Status removeDevice(AudioDevice* device);
    
    // Default device management
    Status setDefaultPlaybackDevice(AudioDevice* device);
    Status setDefaultCaptureDevice(AudioDevice* device);
    AudioDevice* getDefaultPlaybackDevice() const;
    AudioDevice* getDefaultCaptureDevice() const;
    
    // Volume control
    void setMasterVolume(float volume);
    float getMasterVolume() const;
    void setMuted(bool mute);
    bool isMuted() const;
    
    // Audio processing
    Status mixAudio(const void* input, void* output, size_t samples);
    Status applyEffects(void* buffer, size_t samples);
    
    // Format conversion
    Status convertFormat(const void* input, AudioFormat inputFormat,
                        void* output, AudioFormat outputFormat,
                        size_t samples);
};

} // namespace Audio