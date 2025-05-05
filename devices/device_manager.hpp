
#pragma once
#include "../include/types.hpp"

namespace Devices {

enum class DeviceClass {
    Storage,
    Network,
    Display,
    Input,
    Audio,
    Serial,
    USB,
    Generic
};

struct DeviceInfo {
    char name[64];
    DeviceClass class_;
    uint16_t vendorId;
    uint16_t deviceId;
    uint32_t flags;
    void* driverData;
};

class DeviceManager {
private:
    struct DeviceNode {
        DeviceInfo info;
        void* driver;
        DeviceNode* next;
    };

    DeviceNode* devices;
    size_t deviceCount;
    Spinlock lock;

public:
    Status initialize();
    Status enumerateDevices();
    Status loadDriver(const DeviceInfo* device);
    
    // Device operations
    Status registerDevice(const DeviceInfo* info, void** handle);
    Status unregisterDevice(void* handle);
    Status getDeviceInfo(void* handle, DeviceInfo* info);
    
    // Driver management
    Status loadDrivers();
    Status unloadDrivers();
    Status suspendDevices();
    Status resumeDevices();
};

} // namespace Devices