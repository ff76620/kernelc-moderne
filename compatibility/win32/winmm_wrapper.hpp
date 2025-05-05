#pragma once
#include 
#include "../multimedia/audio_manager.hpp"
#include "../multimedia/media_manager.hpp"

namespace Win32Compat {

class WinMMWrapper {
private:
    AudioManager* audioManager;
    MediaManager* mediaManager;

public:
    WinMMWrapper() {
        audioManager = AudioManager::getInstance();
        mediaManager = MediaManager::getInstance();
    }

    MMRESULT waveOutOpen(
        LPHWAVEOUT phwo,
        UINT uDeviceID,
        LPWAVEFORMATEX pwfx,
        DWORD_PTR dwCallback,
        DWORD_PTR dwInstance,
        DWORD fdwOpen
    ) {
        AudioDeviceParams params;
        params.format = convertWaveFormat(pwfx);
        params.callback = convertCallback(dwCallback);
        
        return audioManager->openAudioDevice(params, phwo);
    }

    MMRESULT waveOutWrite(
        HWAVEOUT hwo,
        LPWAVEHDR pwh,
        UINT cbwh
    ) {
        AudioBufferParams params;
        params.data = pwh->lpData;
        params.size = pwh->dwBufferLength;
        
        return audioManager->writeAudioData(hwo, params);
    }

    MCIERROR mciSendStringW(
        LPCWSTR lpszCommand,
        LPWSTR lpszReturnString,
        UINT cchReturn,
        HANDLE hwndCallback
    ) {
        MediaCommandParams params;
        params.command = convertToNativeString(lpszCommand);
        params.callback = hwndCallback;
        
        return mediaManager->executeCommand(params, lpszReturnString, cchReturn);
    }
};

} // namespace Win32Compat