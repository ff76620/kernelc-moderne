
#pragma once
#include "../include/types.hpp"

namespace Power {

enum class PowerState {
    Running,
    Idle,
    Standby,
    Suspend,
    Hibernate,
    PowerOff
};

struct PowerEvent {
    enum class Type {
        ACPlugged,
        ACUnplugged,
        BatteryLow,
        BatteryCritical,
        ThermalWarning,
        ThermalCritical,
        UserIdle,
        LidClosed,
        LidOpened
    } type;
    
    uint64_t timestamp;
    void* data;
};

class PowerManager {
private:
    PowerState currentState;
    PowerState targetState;
    uint8_t batteryLevel;
    bool acConnected;
    bool lidClosed;
    Spinlock lock;

public:
    Status initialize();
    
    // Power state management
    Status transitionTo(PowerState state);
    Status getCurrentState(PowerState* state);
    Status cancelTransition();
    
    // Power events
    Status handlePowerEvent(const PowerEvent* event);
    Status registerEventHandler(void (*handler)(const PowerEvent*));
    
    // Battery management
    Status getBatteryLevel(uint8_t* level);
    Status getBatteryInfo(void* info, size_t size);
    Status isACConnected(bool* connected);
    
    // Thermal management
    Status getThermalInfo(void* info, size_t size);
    Status setThermalPolicy(const void* policy, size_t size);
    
    // Power saving features
    Status setPerformanceProfile(const char* profile);
    Status configurePowerSaving(const void* config, size_t size);
};

} // namespace Power