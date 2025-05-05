#pragma once
#include 
#include "../drivers/input/gamepad_driver.hpp"
#include "../drivers/input/input_manager.hpp"

namespace Win32Compat {
  class DirectInputWrapper {
  private:
    GamepadDriver* gamepadDriver;
    InputManager* inputManager;

  public:
    DirectInputWrapper() {
      gamepadDriver = GamepadDriver::getInstance();
      inputManager = InputManager::getInstance();
    }

    HRESULT DirectInput8Create(
      HINSTANCE hinst,
      DWORD dwVersion,
      REFIID riidltf,
      LPVOID* ppvOut,
      LPUNKNOWN punkOuter
    ) {
      // Create DirectInput interface
      *ppvOut = createDirectInputInterface();
      return S_OK;
    }

    HRESULT CreateDevice(
      REFGUID rguid,
      LPDIRECTINPUTDEVICE* lplpDirectInputDevice,
      LPUNKNOWN pUnkOuter
    ) {
      if(rguid == GUID_SysKeyboard) {
        *lplpDirectInputDevice = createKeyboardDevice();
      }
      else if(rguid == GUID_SysMouse) {
        *lplpDirectInputDevice = createMouseDevice(); 
      }
      else if(rguid == GUID_Joystick) {
        *lplpDirectInputDevice = createJoystickDevice();
      }
      return S_OK;
    }
  };
}