
#pragma once
#include "../drivers/input/gamepad_driver.hpp"
#include 
#include 

namespace Input {

class GamepadManager {
private:
    static constexpr size_t MAX_GAMEPADS = 4;
    std::vector> gamepads;
    bool hotplugEnabled;
    
public:
    GamepadManager();
    ~GamepadManager();
    
    Status initialize();
    Status shutdown();
    
    Status pollDevices();
    Status processEvents();
    
    // Device management
    Status addGamepad(std::unique_ptr gamepad);
    Status removeGamepad(size_t index);
    GamepadDriver* getGamepad(size_t index);
    
    // Hot-plug support
    Status enableHotplug(bool enable);
    bool isHotplugEnabled() const { return hotplugEnabled; }
    
    // Utility methods
    size_t getConnectedGamepads() const;
    bool isGamepadConnected(size_t index) const;
};

} // namespace Input