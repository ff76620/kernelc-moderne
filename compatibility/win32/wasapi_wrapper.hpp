namespace Win32Compat {
class WASAPIWrapper {
    private:
        AudioManager* audioManager;
        
    public:
        WASAPIWrapper() {
            audioManager = AudioManager::getInstance();
        }

        HRESULT ActivateAudioInterface(
            LPCWSTR deviceId,
            REFIID riid,
            void** ppInterface
        ) {
            AudioInterfaceParams params;
            params.deviceId = deviceId;
            params.riid = riid;
            
            return audioManager->activateAudioInterface(params, ppInterface);
        }

        HRESULT InitializeAudioClient(
            IAudioClient* audioClient,
            const WAVEFORMATEX* format,
            AUDCLNT_SHAREMODE shareMode,
            DWORD streamFlags,
            REFERENCE_TIME bufferDuration,
            REFERENCE_TIME periodicity
        ) {
            AudioClientParams params;
            params.format = format;
            params.shareMode = shareMode;
            params.streamFlags = streamFlags;
            params.bufferDuration = bufferDuration;
            params.periodicity = periodicity;
            
            return audioManager->initializeAudioClient(audioClient, params);
        }
};
}