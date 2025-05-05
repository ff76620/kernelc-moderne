
#pragma once
#include "audio_device.hpp"
#include 
#include 

namespace Audio {

class CodecManager {
private:
    std::unordered_map> codecs;
    
public:
    Status initialize();
    Status shutdown();
    
    // Codec management
    Status registerCodec(AudioFormat format, AudioCodec* codec);
    Status unregisterCodec(AudioFormat format);
    AudioCodec* getCodec(AudioFormat format);
    
    // Format conversion
    Status convertFormat(const void* input, AudioFormat inputFormat,
                        void* output, AudioFormat outputFormat,
                        size_t& size);
                        
    // Format detection
    AudioFormat detectFormat(const void* data, size_t size);
    bool isFormatSupported(AudioFormat format) const;
    
    // Playlist handling
    Status parsePlaylist(const char* filename, std::vector& tracks);
};

} // namespace Audio