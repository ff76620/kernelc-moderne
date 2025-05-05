
#pragma once
#include "../drivers/input/input_device.hpp"

namespace Input {

struct MouseState {
    int32_t x;
    int32_t y;
    int32_t deltaX;
    int32_t deltaY;
    int32_t scrollX;
    int32_t scrollY;
    uint32_t buttons;
};

class MouseDriver : public InputDevice {
private:
    MouseState currentState;
    MouseState previousState;
    bool absolute;
    uint32_t dpi;
    float sensitivity;
    
public:
    MouseDriver();
    virtual ~MouseDriver();
    
    virtual Status initialize() override;
    virtual Status shutdown() override;
    
    virtual Status poll() override;
    virtual Status flush() override;
    
    virtual Status processEvent(const InputEvent& event) override;
    
    // Mouse specific methods
    const MouseState& getState() const;
    Status setPosition(int32_t x, int32_t y);
    Status setSensitivity(float sensitivity);
    Status setDPI(uint32_t dpi);
    
    bool isButtonPressed(uint32_t button) const;
    bool isButtonReleased(uint32_t button) const;
    bool wasButtonPressed(uint32_t button) const;
};

} // namespace Input