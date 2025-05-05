#pragma once
#include 
#include "../drivers/audio/audio_manager.hpp"
#include "../multimedia/media_manager.hpp"

namespace Win32Compat {

class DirectSoundWrapper {
private:
    AudioManager* audioManager;
    MediaManager* mediaManager;

public:
    DirectSoundWrapper() {
        audioManager = AudioManager::getInstance();
        mediaManager = MediaManager::getInstance();
    }

    HRESULT DirectSoundCreate8(
        LPCGUID pcGuidDevice,
        LPDIRECTSOUND8* ppDS8,
        LPUNKNOWN pUnkOuter
    ) {
        AudioDeviceParams params;
        params.deviceId = convertGUID(pcGuidDevice);
        
        return audioManager->createDirectSound(params, ppDS8);
    }

    HRESULT CreateSoundBuffer(
        LPCDSBUFFERDESC pcDSBufferDesc,
        LPDIRECTSOUNDBUFFER* ppDSBuffer,
        LPUNKNOWN pUnkOuter
    ) {
        BufferParams params;
        params.format = pcDSBufferDesc->lpwfxFormat;
        params.size = pcDSBufferDesc->dwBufferBytes;
        params.flags = pcDSBufferDesc->dwFlags;

        return audioManager->createSoundBuffer(params, ppDSBuffer);
    }
};

} // namespace Win32Compat