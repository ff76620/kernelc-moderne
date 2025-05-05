
#pragma once
#include "../include/types.hpp"

namespace Audio {

enum class AudioFormat {
    // Raw PCM formats
    PCM_8,
    PCM_16,
    PCM_24,
    PCM_32,
    FLOAT_32,
    FLOAT_64,
    
    // Compressed formats
    MP3,
    AAC,
    FLAC,
    OGG,
    WAV,
    WMA,
    
    // Playlist format
    M3U
};

struct AudioCodec {
    virtual ~AudioCodec() = default;
    virtual Status decode(const void* input, size_t inputSize,
                         void* output, size_t& outputSize) = 0;
    virtual Status encode(const void* input, size_t inputSize,
                         void* output, size_t& outputSize) = 0;
    virtual AudioFormat getFormat() const = 0;
};

class CodecFactory {
public:
    static AudioCodec* createCodec(AudioFormat format);
    static void destroyCodec(AudioCodec* codec);
    static bool isFormatSupported(AudioFormat format);
};

} // namespace Audio