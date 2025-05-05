
#pragma once
#include "../drivers/io/keyboard_manager.hpp"
#include "../drivers/io/mouse_manager.hpp"

namespace IO {

struct InputDevice {
    char name[64];
    uint32_t vendorId;
    uint32_t productId;
    bool isAbsolute;
    void* driverData;
};

class InputManager {
private:
    static InputDevice* devices;
    static size_t deviceCount;
    static Spinlock lock;
    
    KeyboardManager keyboardMgr;
    MouseManager mouseMgr;

public:
    Status initialize();
    
    // Device management
    Status enumerateDevices();
    Status registerDevice(InputDevice* device);
    Status unregisterDevice(InputDevice* device);
    
    // Event handling
    Status processEvents();
    Status registerEventHandler(void (*handler)(const KeyEvent*));
    Status registerEventHandler(void (*handler)(const MouseEvent*));
    
    // Access to specific managers
    KeyboardManager& getKeyboardManager() { return keyboardMgr; }
    MouseManager& getMouseManager() { return mouseMgr; }
    
    // Device properties
    Status getDeviceInfo(InputDevice* device, void* info, size_t size);
    Status setDeviceProperty(InputDevice* device, const char* property, const void* value);
};

} // namespace IO