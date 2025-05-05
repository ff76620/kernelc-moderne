#include "../drivers/DriverManager.hpp"
#include "../loggin/EventLogger.hpp"

namespace Kernel {

DriverManager& DriverManager::getInstance() {
    static DriverManager instance;
    return instance;
}

bool DriverManager::initialize() {
    EventLogger::log("Initializing Driver Manager...");
    
    try {
        // Initialize core drivers here
        status = Status::OK;
        EventLogger::log("Driver Manager initialized successfully");
        return true;
    } 
    catch (const std::exception& e) {
        EventLogger::log("Error initializing Driver Manager: " + std::string(e.what()));
        status = Status::ERROR;
        return false;
    }
}

void DriverManager::shutdown() {
    EventLogger::log("Shutting down Driver Manager...");
    
    for (auto& pair : drivers) {
        try {
            pair.second->shutdown();
        }
        catch (const std::exception& e) {
            EventLogger::log("Error shutting down driver " + pair.first + ": " + e.what());
        }
    }
    
    drivers.clear();
    status = Status::NOT_INITIALIZED;
    EventLogger::log("Driver Manager shutdown complete");
}

void DriverManager::unregisterDriver(const std::string& name) {
    auto it = drivers.find(name);
    if (it != drivers.end()) {
        try {
            it->second->shutdown();
            drivers.erase(it);
            EventLogger::log("Driver " + name + " unregistered successfully");
        }
        catch (const std::exception& e) {
            EventLogger::log("Error unregistering driver " + name + ": " + e.what());
        }
    }
}

} // namespace Kernel