#pragma once
#include 
#include "../drivers/input/input_manager.hpp"

namespace Win32Compat {
class RawInputWrapper {
private:
    InputManager* inputManager;

public:
    RawInputWrapper() {
        inputManager = InputManager::getInstance();
    }

    BOOL RegisterRawInputDevices(
        PCRAWINPUTDEVICE pRawInputDevices,
        UINT uiNumDevices,
        UINT cbSize
    ) {
		 RawInputParams params;
            params.devices = pRawInputDevices;
            params.numDevices = uiNumDevices;
            params.structSize = cbSize;
			
        return inputManager->registerRawDevices(pRawInputDevices, uiNumDevices);
    }

    UINT GetRawInputData(
        HRAWINPUT hRawInput,
        UINT uiCommand,
        LPVOID pData,
        PUINT pcbSize,
        UINT cbSizeHeader
    ) {
        return inputManager->getRawInputData(hRawInput, uiCommand, pData, pcbSize);
    }
};
} // namespace Win32Compat