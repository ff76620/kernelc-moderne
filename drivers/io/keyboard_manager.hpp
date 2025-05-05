
#pragma once
#include "../include/types.hpp"

namespace IO {

enum class KeyCode {
    None = 0,
    // Letters
    A = 4, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    // Numbers
    Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9, Num0,
    // Function keys
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    // Special keys
    Enter, Escape, Backspace, Tab, Space, Delete,
    // Modifiers
    LShift, RShift, LCtrl, RCtrl, LAlt, RAlt, 
    // Arrow keys
    Up, Down, Left, Right
};

struct KeyEvent {
    KeyCode code;
    bool pressed;
    bool shift;
    bool ctrl;
    bool alt;
    uint64_t timestamp;
};

class KeyboardManager {
private:
    static KeyEvent eventBuffer[256];
    static size_t eventCount;
    static Spinlock lock;
    static bool numLock;
    static bool capsLock;
    static bool scrollLock;

public:
    Status initialize();
    Status handleInterrupt();
    Status getEvent(KeyEvent* event);
    Status flushEvents();
    
    bool isKeyPressed(KeyCode code);
    bool isModifierActive(KeyCode modifier);
    
    // LED control
    void updateLEDs();
    void setLED(bool numLock, bool capsLock, bool scrollLock);
    
    // Layout management
    Status setKeyboardLayout(const char* layout);
    const char* getCurrentLayout();
};

} // namespace IO