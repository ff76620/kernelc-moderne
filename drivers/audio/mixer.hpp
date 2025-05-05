
#pragma once
#include "audio_device.hpp"

namespace Audio {

struct MixerChannel {
    bool active;
    float volume;
    bool muted;
    void* buffer;
    size_t bufferSize;
};

class AudioMixer {
private:
    static const size_t MAX_CHANNELS = 32;
    MixerChannel channels[MAX_CHANNELS];
    size_t activeChannels;
    
    float masterVolume;
    bool masterMute;
    
public:
    Status initialize();
    Status shutdown();
    
    // Channel management
    int32_t allocateChannel();
    Status freeChannel(int32_t channel);
    Status clearChannel(int32_t channel);
    
    // Channel control
    Status setChannelVolume(int32_t channel, float volume);
    Status setChannelMute(int32_t channel, bool mute);
    Status writeToChannel(int32_t channel, const void* data, size_t size);
    
    // Mixing
    Status mix(void* output, size_t samples);
    Status applyEffects(int32_t channel, void* data, size_t samples);
    
    // Master controls
    void setMasterVolume(float volume);
    float getMasterVolume() const;
    void setMasterMute(bool mute);
    bool isMasterMuted() const;
};

} // namespace Audio