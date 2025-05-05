#pragma once
#include 
#include "../drivers/input/gamepad_driver.hpp"

namespace Win32Compat {
class XInputWrapper {
private:
    GamepadDriver* gamepadDriver;

public:
    XInputWrapper() {
        gamepadDriver = GamepadDriver::getInstance();
    }

    DWORD XInputGetState(
        DWORD dwUserIndex,
        XINPUT_STATE* pState
    ) {
        return gamepadDriver->getGamepadState(dwUserIndex, pState);
    }

    DWORD XInputSetState(
        DWORD dwUserIndex,
        XINPUT_VIBRATION* pVibration
    ) {
        return gamepadDriver->setVibration(dwUserIndex, pVibration);
    }
};
} // namespace Win32Compat
      