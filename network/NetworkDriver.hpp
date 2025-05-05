#ifndef NETWORK_DRIVER_HPP
#define NETWORK_DRIVER_HPP

#include "../include/types.hpp"
#include 
#include 
#include 
#include 

namespace Kernel {
namespace Network {

enum class NetworkProtocol {
    TCP,
    UDP,
    ICMP,
    ARP
};

struct NetworkPacket {
    std::vector data;
    size_t size;
    NetworkProtocol protocol;
    std::string sourceAddress;
    std::string destinationAddress;
    uint16_t sourcePort;
    uint16_t destinationPort;
};

class NetworkDriver {
public:
    static NetworkDriver& getInstance();
    
    bool initialize();
    void shutdown();
    
    // Network operations
    bool sendPacket(const NetworkPacket& packet);
    bool receivePacket(NetworkPacket& packet);
    
    // Interface management
    bool configureInterface(const std::string& interface, const std::string& ipAddress);
    bool enableInterface(const std::string& interface);
    bool disableInterface(const std::string& interface);
    
    // Connection management
    bool openConnection(const std::string& address, uint16_t port);
    bool closeConnection(const std::string& address, uint16_t port);
    
    // Status and metrics
    bool isInterfaceUp(const std::string& interface);
    size_t getTransmitQueueLength();
    size_t getReceiveQueueLength();
    float getBandwidthUtilization();

private:
    NetworkDriver();
    ~NetworkDriver();
    
    std::mutex mutex;
    std::vector activeInterfaces;
    bool initialized;
    
    // Internal methods
    bool validatePacket(const NetworkPacket& packet);
    bool enqueuePacket(const NetworkPacket& packet);
    bool processReceivedPacket();
    
    // Prevent copying
    NetworkDriver(const NetworkDriver&) = delete;
    NetworkDriver& operator=(const NetworkDriver&) = delete;
};

} // namespace Network
} // namespace Kernel

#endif