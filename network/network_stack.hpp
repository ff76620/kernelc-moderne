#ifndef NETWORK_STACK_HPP
#define NETWORK_STACK_HPP

#include 
#include 
#include 
#include "../include/types.hpp"

namespace Kernel {
namespace Network {

class NetworkStack {
public:
    NetworkStack();
    ~NetworkStack();

    bool initialize();
    void shutdown();

    // Connection management
    ConnectionID createConnection(const std::string& address, uint16_t port);
    bool closeConnection(ConnectionID id);
    
    // Data transmission
    bool sendData(ConnectionID id, const void* data, size_t size);
    bool receiveData(ConnectionID id, void* buffer, size_t size);

    // Network configuration
    void setInterface(const std::string& interface);
    void configureProtocol(ProtocolType type, const ProtocolConfig& config);
    
    // Security
    void enableEncryption(bool enable);
    void setSecurityLevel(SecurityLevel level);

    // Monitoring
    struct NetworkStats {
        uint64_t bytesSent;
        uint64_t bytesReceived;
        uint32_t activeConnections;
        double bandwidth;
        uint32_t packetLoss;
        double latency;
    };

    NetworkStats getStats() const;

private:
    std::vector connections;
    SecurityConfig securityConfig;
    NetworkConfig networkConfig;

    void handlePackets();
    void monitorPerformance();
    void manageConnections();
};

} // namespace Network
} // namespace Kernel

#endif