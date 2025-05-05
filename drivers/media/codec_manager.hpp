
#pragma once
#include "../include/types.hpp"

namespace Media {

enum class CodecType {
    AudioEncoder,
    AudioDecoder,
    VideoEncoder,
    VideoDecoder
};

struct Codec {
    char name[64];
    CodecType type;
    const char* mimeType;
    void* implementation;
};

class CodecManager {
private:
    static Codec* codecs;
    static size_t codecCount;
    static Spinlock lock;

public:
    Status initialize();
    
    // Codec management
    Status registerCodec(const Codec* codec);
    Status unregisterCodec(const Codec* codec);
    Status findCodec(const char* mimeType, CodecType type, Codec** codec);
    
    // Codec operations
    Status createInstance(const Codec* codec, void** instance);
    Status destroyInstance(void* instance);
    
    // Encoding/Decoding
    Status encode(void* instance, const void* input, size_t inputSize, 
                 void* output, size_t* outputSize);
    Status decode(void* instance, const void* input, size_t inputSize,
                 void* output, size_t* outputSize);
                 
    // Codec properties
    Status getCodecInfo(const Codec* codec, void* info, size_t size);
    Status setCodecParameters(void* instance, const void* params, size_t size);
};

} // namespace Media