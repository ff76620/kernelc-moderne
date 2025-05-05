      #pragma once
        #include 
        #include 
        #include 
        #include 
        #include "../drivers/input/input_manager.hpp"
        #include "../drivers/audio/audio_manager.hpp"

        namespace Win32Compat {
        class IOImplementation {
        private:
            InputManager* inputManager;
            AudioManager* audioManager;
            
        public:
            // DirectInput
            HRESULT CreateInputDevice(
                const InputDeviceParams& params,
                IDirectInputDevice8** ppDevice
            ) {
                return inputManager->createDevice(params, ppDevice);
            }

            // XInput Support
            DWORD XInputGetState(
                const XInputParams& params,
                XINPUT_STATE* pState
            ) {
                return inputManager->getXInputState(params, pState);
            }

            // DirectSound
            HRESULT CreateSoundDevice(
                const AudioDeviceParams& params,
                IDirectSound8** ppDS8
            ) {
                return audioManager->createDirectSoundDevice(params, ppDS8);
            }

            // XAudio2
            HRESULT CreateXAudio2(
                const XAudio2Params& params,
                IXAudio2** ppXAudio2
            ) {
                return audioManager->createXAudio2Device(params, ppXAudio2);
            }
        };
        }
      