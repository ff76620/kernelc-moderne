
#pragma once
#include "../include/types.hpp"
#include 

namespace Input {

enum class InputType {
    KEYBOARD,
    MOUSE,
    GAMEPAD,
    TOUCHSCREEN,
    GENERIC
};

struct InputEvent {
    uint32_t timestamp;
    uint16_t type;
    uint16_t code;
    int32_t value;
};

class InputDevice {
protected:
    char name[64];
    InputType type;
    bool connected;
    std::vector eventQueue;

public:
    virtual ~InputDevice() = default;
    
    virtual Status initialize() = 0;
    virtual Status shutdown() = 0;
    
    virtual Status poll() = 0;
    virtual Status flush() = 0;
    
    virtual Status processEvent(const InputEvent& event) = 0;
    virtual Status queueEvent(const InputEvent& event);
    
    virtual bool hasEvents() const;
    virtual Status getNextEvent(InputEvent& event);
    
    InputType getType() const { return type; }
    const char* getName() const { return name; }
    bool isConnected() const { return connected; }
};

} // namespace Input