
#pragma once
#include "../include/types.hpp"

namespace Power {

enum class PowerState {
    Running,
    Idle,
    Sleep,
    Hibernate,
    Shutdown
};

struct PowerEvent {
    PowerState state;
    uint64_t timestamp;
    uint32_t flags;
};

class PowerManager {
private:
    PowerState currentState;
    uint32_t batteryLevel;
    bool onACPower;
    Spinlock lock;

public:
    Status initialize();
    Status setPowerState(PowerState state);
    Status getPowerState(PowerState* state);
    
    // Power management
    Status handlePowerEvent(const PowerEvent* event);
    Status getBatteryStatus(uint32_t* level, bool* charging);
    Status configurePowerSaving(uint32_t flags);
    
    // System power operations
    Status suspend();
    Status resume();
    Status hibernate();
    Status shutdown();
    Status reboot();
};

} // namespace Power