#pragma once
#include 
#include "../drivers/audio/audio_manager.hpp"
#include "../drivers/audio/extended_audio_device.hpp"
#include "../multimedia/effects_manager.hpp"

namespace Win32Compat {
class XAudio2Wrapper {
private:
    AudioManager* audioManager;
    EffectsManager* effectsManager;
    ExtendedAudioDevice* audioDevice;

public:
    XAudio2Wrapper() {
        audioManager = AudioManager::getInstance();
        effectsManager = EffectsManager::getInstance();
        audioDevice = ExtendedAudioDevice::getInstance();
    }

    HRESULT XAudio2Create(
        IXAudio2** ppXAudio2,
        UINT32 Flags,
        XAUDIO2_PROCESSOR XAudio2Processor
    ) {
        return audioManager->createXAudio2Instance(ppXAudio2, Flags);
    }

    HRESULT CreateMasteringVoice(
        IXAudio2MasteringVoice** ppMasteringVoice,
        UINT32 InputChannels = XAUDIO2_DEFAULT_CHANNELS,
        UINT32 InputSampleRate = XAUDIO2_DEFAULT_SAMPLERATE,
        UINT32 Flags = 0,
        LPCWSTR szDeviceId = NULL,
        const XAUDIO2_EFFECT_CHAIN* pEffectChain = NULL,
        AUDIO_STREAM_CATEGORY StreamCategory = AudioCategory_GameEffects
    ) {
        AudioDeviceParams params;
        params.channels = InputChannels;
        params.sampleRate = InputSampleRate;
        params.flags = Flags;
        params.deviceId = szDeviceId;
        
        if(pEffectChain) {
            params.effectChain = effectsManager->convertEffectChain(pEffectChain);
        }
        
        return audioManager->createMasteringVoice(ppMasteringVoice, params);
    }

    HRESULT CreateSourceVoice(
        IXAudio2SourceVoice** ppSourceVoice,
        const WAVEFORMATEX* pSourceFormat,
        UINT32 Flags = 0,
        float MaxFrequencyRatio = XAUDIO2_DEFAULT_FREQ_RATIO,
        IXAudio2VoiceCallback* pCallback = NULL,
        const XAUDIO2_VOICE_SENDS* pSendList = NULL,
        const XAUDIO2_EFFECT_CHAIN* pEffectChain = NULL
    ) {
        SourceVoiceParams params;
        params.format = audioDevice->convertWaveFormat(pSourceFormat);
        params.flags = Flags;
        params.freqRatio = MaxFrequencyRatio;
        params.callback = pCallback;
        
        if(pSendList) {
            params.sendList = audioDevice->convertVoiceSends(pSendList);
        }
        
        if(pEffectChain) {
            params.effectChain = effectsManager->convertEffectChain(pEffectChain);
        }
        
        return audioManager->createSourceVoice(ppSourceVoice, params);
    }
};
} // namespace Win32Compat