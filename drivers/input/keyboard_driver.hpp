
#pragma once
#include "../drivers/input/input_device.hpp"
#include 

namespace Input {

enum class KeyState {
    RELEASED,
    PRESSED,
    REPEATED
};

class KeyboardDriver : public InputDevice {
private:
    static constexpr size_t MAX_KEYS = 256;
    std::array keyStates;
    bool numLock;
    bool capsLock;
    bool scrollLock;
    
public:
    KeyboardDriver();
    virtual ~KeyboardDriver();
    
    virtual Status initialize() override;
    virtual Status shutdown() override;
    
    virtual Status poll() override;
    virtual Status flush() override;
    
    virtual Status processEvent(const InputEvent& event) override;
    
    // Keyboard specific methods
    bool isKeyPressed(uint16_t keyCode) const;
    KeyState getKeyState(uint16_t keyCode) const;
    bool isModifierActive(uint16_t modifier) const;
    
    // LED control
    Status setLED(bool numLock, bool capsLock, bool scrollLock);
    Status updateLEDState();
};

} // namespace Input