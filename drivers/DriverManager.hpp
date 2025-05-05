#ifndef DRIVER_MANAGER_HPP
#define DRIVER_MANAGER_HPP

#include "../include/types.hpp"
#include 
#include 
#include 
#include 

namespace Kernel {

class Driver {
public:
    virtual ~Driver() = default;
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    virtual const std::string& getName() const = 0;
};

class DriverManager {
public:
    static DriverManager& getInstance();

    bool initialize();
    void shutdown();
    
    template
    T* registerDriver(Args&&... args) {
        auto driver = std::make_unique(std::forward(args)...);
        if (!driver->initialize()) {
            return nullptr;
        }
        T* raw_ptr = driver.get();
        drivers[driver->getName()] = std::move(driver);
        return raw_ptr;
    }

    template
    T* getDriver(const std::string& name) {
        auto it = drivers.find(name);
        if (it != drivers.end()) {
            return dynamic_cast(it->second.get());
        }
        return nullptr;
    }

    void unregisterDriver(const std::string& name);
    
    enum class Status {
        OK,
        ERROR,
        NOT_INITIALIZED
    };

    Status getStatus() const { return status; }

private:
    DriverManager() : status(Status::NOT_INITIALIZED) {}
    ~DriverManager() = default;

    std::unordered_map> drivers;
    Status status;

    // Prevent copying
    DriverManager(const DriverManager&) = delete;
    DriverManager& operator=(const DriverManager&) = delete;
};

} // namespace Kernel

#endif