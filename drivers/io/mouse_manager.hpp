
#pragma once
#include "../include/types.hpp"

namespace IO {

struct MouseEvent {
    int32_t deltaX;
    int32_t deltaY;
    int32_t deltaZ;  // Scroll
    bool leftButton;
    bool rightButton;
    bool middleButton;
    uint64_t timestamp;
};

class MouseManager {
private:
    static MouseEvent eventBuffer[256];
    static size_t eventCount;
    static Spinlock lock;
    
    int32_t currentX;
    int32_t currentY;
    uint32_t sensitivity;
    bool absolute;

public:
    Status initialize();
    Status handleInterrupt();
    Status getEvent(MouseEvent* event);
    Status flushEvents();
    
    // Position management
    void getPosition(int32_t* x, int32_t* y);
    void setPosition(int32_t x, int32_t y);
    
    // Settings
    void setSensitivity(uint32_t value);
    void setAbsoluteMode(bool enable);
    
    // Button state
    bool isButtonPressed(uint8_t button);
    uint32_t getButtonState();
};

} // namespace IO