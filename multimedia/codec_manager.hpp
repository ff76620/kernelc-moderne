
#pragma once
#include "../include/types.hpp"

namespace Multimedia {

enum class CodecType {
    Audio,
    Video
};

struct CodecInfo {
    char name[32];
    CodecType type;
    uint32_t version;
    uint32_t flags;
    void* private_data;
};

class CodecManager {
private:
    struct Codec {
        CodecInfo info;
        void* handle;
        void* context;
    };

    Codec* codecs;
    size_t codecCount;
    Spinlock lock;

public:
    Status initialize();
    Status loadCodecs();
    
    // Codec operations
    Status findCodec(const char* name, CodecType type, Codec** codec);
    Status createContext(Codec* codec, void** context);
    Status destroyContext(void* context);
    
    // Encoding/Decoding
    Status encode(void* context, const void* input, size_t inSize, void* output, size_t* outSize);
    Status decode(void* context, const void* input, size_t inSize, void* output, size_t* outSize);
    
    // Codec management
    Status registerCodec(const CodecInfo* info);
    Status unregisterCodec(const char* name);
};

} // namespace Multimedia