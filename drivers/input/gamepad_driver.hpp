
#pragma once
#include "../drivers/input/input_device.hpp"
#include 

namespace Input {

enum class Status {
    Success,
    DeviceNotFound,
    InitializationError,
    CommunicationError
};

struct GamepadState {
    uint32_t buttons;      // Bit field for button states
    float leftStickX;      // -1.0 to 1.0
    float leftStickY;      // -1.0 to 1.0
    float rightStickX;     // -1.0 to 1.0
    float rightStickY;     // -1.0 to 1.0
    float leftTrigger;     // 0.0 to 1.0
    float rightTrigger;    // 0.0 to 1.0
    float leftVibration;   // 0.0 to 1.0
    float rightVibration;  // 0.0 to 1.0
};

class GamepadDriver {
public:
    GamepadDriver();
    ~GamepadDriver();

    // Initialization
    Status initialize();
    Status shutdown();
    
    // Input handling
    Status poll();
    Status setDeadzone(float value);
    
    // Button state queries
    bool isButtonPressed(uint32_t button) const;
    bool isButtonReleased(uint32_t button) const;
    bool wasButtonPressed(uint32_t button) const;
    
    // Force feedback
    Status setVibration(float leftMotor, float rightMotor);
    
    // Battery management
    Status updateBatteryLevel();
    int getBatteryLevel() const { return batteryLevel; }
    bool isWireless() const { return wireless; }

private:
    GamepadState currentState;
    GamepadState previousState;
    float deadzone;
    bool wireless;
    int batteryLevel;
};

} // namespace Input