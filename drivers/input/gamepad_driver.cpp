
#include "gamepad_driver.hpp"
#include 

namespace Input {

bool GamepadDriver::checkWirelessStatus() {
    // Query device capabilities
    DeviceCapabilities caps;
    if (!queryDeviceCapabilities(&caps)) {
        return false;
    }
    
    return caps.features & FEATURE_WIRELESS;
}

void GamepadDriver::updateBatteryLevel() {
    if (!wireless) {
        batteryLevel = 0;
        return;
    }
    
    // Query battery status from device
    BatteryStatus status;
    if (queryBatteryStatus(&status)) {
        batteryLevel = status.level;
    }
}

bool GamepadDriver::readInputData() {
    RawInputData rawData;
    if (!readRawData(&rawData)) {
        return false;
    }
    
    // Convert raw data to current state
    currentState.leftStickX = normalizeAxis(rawData.lx);
    currentState.leftStickY = normalizeAxis(rawData.ly);
    currentState.rightStickX = normalizeAxis(rawData.rx);
    currentState.rightStickY = normalizeAxis(rawData.ry);
    
    currentState.leftTrigger = normalizeTrigger(rawData.lt);
    currentState.rightTrigger = normalizeTrigger(rawData.rt);
    
    currentState.buttons = rawData.buttons;
    
    // Update wireless status if needed
    if (wireless && ++wirelessCheckCounter >= WIRELESS_CHECK_INTERVAL) {
        wirelessCheckCounter = 0;
        updateBatteryLevel();
        
        if (batteryLevel < LOW_BATTERY_THRESHOLD) {
            notifyLowBattery();
        }
    }
    
    return true;
}

float GamepadDriver::normalizeAxis(int16_t raw) {
    return std::clamp(static_cast(raw) / 32768.0f, -1.0f, 1.0f);
}

float GamepadDriver::normalizeTrigger(uint8_t raw) {
    return static_cast(raw) / 255.0f;
}

void GamepadDriver::shutdown() {
    if (wireless) {
        // Send shutdown command to wireless controller
        sendShutdownCommand();
    }
    
    // Release device handle
    releaseDevice();
}

} // namespace Input