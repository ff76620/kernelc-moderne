#pragma once
#include 
#include 
#include "../core/kernel.hpp"
#include "../drivers/input/keyboard_driver.hpp"
#include "../drivers/input/mouse_driver.hpp"
#include "../drivers/input/gamepad_driver.hpp"

namespace Win32Compat {

class InputManager {
private:
    static std::unique_ptr instance;
    std::queue messageQueue;
    KeyboardDriver* keyboardDriver;
    MouseDriver* mouseDriver;
    GamepadDriver* gamepadDriver;
    
    InputManager() {
        keyboardDriver = new KeyboardDriver();
        mouseDriver = new MouseDriver();
        gamepadDriver = new GamepadDriver();
    }

public:
    static InputManager* getInstance() {
        if (!instance) {
            instance = std::unique_ptr(new InputManager());
        }
        return instance.get();
    }

    bool getMessage(LPMSG lpMsg, HWND hwnd) {
        if (messageQueue.empty()) {
            pollDevices();
        }
        
        if (!messageQueue.empty()) {
            *lpMsg = messageQueue.front();
            messageQueue.pop();
            return true;
        }
        return false;
    }

    bool translateMessage(const MSG* lpMsg) {
        // Convert hardware messages to character messages
        return true;
    }

private:
    void pollDevices() {
        pollKeyboard();
        pollMouse();
        pollGamepad();
    }

    void pollKeyboard();
    void pollMouse();
    void pollGamepad();
};

} // namespace Win32Compat