
#pragma once
#include "audio_device.hpp"
#include "codec_manager.hpp"

namespace Audio {

class ExtendedAudioDevice : public AudioDevice {
protected:
    CodecManager codecManager;
    std::vector supportedFormats;
    
public:
    Status initialize() override;
    Status shutdown() override;
    
    // Extended format support
    virtual Status playSupportedFormat(const void* data, size_t size, 
                                     AudioFormat format);
    virtual Status recordToFormat(void* buffer, size_t& size, 
                                AudioFormat format);
    
    // Format capabilities
    virtual bool supportsFormat(AudioFormat format) const;
    virtual std::vector getSupportedFormats() const;
    
    // Playlist support
    virtual Status loadPlaylist(const char* filename);
    virtual Status getNextTrack(std::string& track);
    
    // Additional features
    virtual Status applyRealtimeEffects(void* data, size_t size);
    virtual Status setEqualizerSettings(const float* bands, size_t count);
};

} // namespace Audio