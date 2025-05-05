
#pragma once
#include "../include/types.hpp"

namespace Input {

enum class DeviceType {
    Keyboard,
    Mouse,
    Gamepad,
    Touchscreen,
    Tablet,
    Generic
};

struct InputEvent {
    uint32_t deviceId;
    uint32_t type;
    uint32_t code;
    int32_t value;
    uint64_t timestamp;
};

class InputManager {
private:
    struct InputDevice {
        DeviceType type;
        uint32_t id;
        char name[64];
        bool active;
        void* driverData;
    };

    InputDevice* devices;
    size_t deviceCount;
    Spinlock lock;

    // Event queue
    InputEvent* eventQueue;
    size_t queueSize;
    size_t queueHead;
    size_t queueTail;

public:
    Status initialize();
    
    // Device management
    Status registerDevice(DeviceType type, const char* name, InputDevice** device);
    Status unregisterDevice(InputDevice* device);
    Status enumerateDevices();
    
    // Event handling
    Status pushEvent(const InputEvent* event);
    Status pollEvent(InputEvent* event);
    Status flushEvents();
    
    // Device specific operations
    Status setKeyboardLayout(InputDevice* device, const char* layout);
    Status setMouseSensitivity(InputDevice* device, float sensitivity);
    Status calibrateTouchscreen(InputDevice* device);
    
    // Device properties
    Status getDeviceInfo(InputDevice* device, void* info, size_t size);
    Status setDeviceProperty(InputDevice* device, const char* property, const void* value);
};

} // namespace Input