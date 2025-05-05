
#pragma once
#include "../include/types.hpp"

namespace USB {

enum class DeviceSpeed {
    Low,    // 1.5 Mbps
    Full,   // 12 Mbps
    High,   // 480 Mbps
    Super,  // 5 Gbps
    Super20 // 20 Gbps
};

struct USBDevice {
    uint16_t vendorId;
    uint16_t productId;
    uint16_t bcdDevice;
    uint8_t deviceClass;
    uint8_t deviceSubClass;
    uint8_t deviceProtocol;
    DeviceSpeed speed;
    void* driverData;
};

class USBManager {
private:
    struct EndpointConfig {
        uint8_t address;
        uint8_t attributes;
        uint16_t maxPacketSize;
        uint8_t interval;
    };

    USBDevice* devices;
    size_t deviceCount;
    Spinlock lock;

public:
    Status initialize();
    
    // Device management
    Status enumerateDevices();
    Status registerDevice(USBDevice* device);
    Status unregisterDevice(USBDevice* device);
    
    // Device operations
    Status controlTransfer(USBDevice* device, uint8_t request, uint16_t value, uint16_t index, void* data, size_t length);
    Status bulkTransfer(USBDevice* device, uint8_t endpoint, void* data, size_t length, size_t* transferred);
    Status interruptTransfer(USBDevice* device, uint8_t endpoint, void* data, size_t length);
    
    // Endpoint management
    Status configureEndpoint(USBDevice* device, const EndpointConfig* config);
    Status resetEndpoint(USBDevice* device, uint8_t endpoint);
    
    // Power management
    Status suspendDevice(USBDevice* device);
    Status resumeDevice(USBDevice* device);
    Status resetDevice(USBDevice* device);
};

} // namespace USB