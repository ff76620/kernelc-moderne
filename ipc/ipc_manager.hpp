#ifndef IPC_MANAGER_HPP
#define IPC_MANAGER_HPP

#include 
#include 
#include 
#include 
#include 
#include 
#include 
#include 
#include 

namespace Kernel {

using ProcessId = int;

struct Message {
    ProcessId sender;
    ProcessId receiver; 
    std::string data;
    size_t priority;
    uint64_t timestamp;
};

class MessageQueue {
private:
    std::priority_queue messages;
    std::mutex queueMutex;
    
public:
    void optimizedSend(ProcessId dest, const Message& msg) {
        std::lock_guard lock(queueMutex);
        // Copy message and set timestamp
        Message msgCopy = msg;
        msgCopy.timestamp = getCurrentTimestamp();
        messages.push(msgCopy);
        notifyReceiver(dest);
    }
    
    Message receive(ProcessId receiver) {
        std::lock_guard lock(queueMutex);
        if(!messages.empty()) {
            Message msg = messages.top();
            if(msg.receiver == receiver) {
                messages.pop();
                return msg;
            }
        }
        return Message{}; // Return empty message if none found
    }

    bool empty() const {
        std::lock_guard lock(queueMutex);
        return messages.empty();
    }

    void clear() {
        std::lock_guard lock(queueMutex);
        while(!messages.empty()) {
            messages.pop();
        }
    }

private:
    uint64_t getCurrentTimestamp() const {
        return std::chrono::duration_cast(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
    }

    void notifyReceiver(ProcessId dest) {
        // Implementation to notify receiving process
        // This could involve system calls or signals
    }
};

class SharedMemoryManager {
private:
    struct SharedSegment {
        void* address;
        size_t size;
        int refCount;
        std::mutex segmentMutex;
    };
    
    std::unordered_map segments;
    std::mutex managerMutex;
    
public:
    void* allocate(const std::string& name, size_t size) {
        std::lock_guard lock(managerMutex);
        auto it = segments.find(name);
        if(it != segments.end()) {
            std::lock_guard segLock(it->second.segmentMutex);
            it->second.refCount++;
            return it->second.address;
        }

        void* addr = mmap(nullptr, size, PROT_READ | PROT_WRITE, 
                         MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        if(addr == MAP_FAILED) {
            return nullptr;
        }

        segments[name] = {addr, size, 1};
        return addr;
    }
    
    bool deallocate(const std::string& name) {
        std::lock_guard lock(managerMutex);
        auto it = segments.find(name);
        if(it == segments.end()) {
            return false;
        }

        std::lock_guard segLock(it->second.segmentMutex);
        if(--it->second.refCount <= 0) {
            munmap(it->second.address, it->second.size);
            segments.erase(it);
        }
        return true;
    }

    ~SharedMemoryManager() {
        std::lock_guard lock(managerMutex);
        for(auto& [name, segment] : segments) {
            if(segment.address) {
                munmap(segment.address, segment.size);
                segment.address = nullptr;
            }
        }
        segments.clear();
    }

    std::vector> getSegments() const {
        std::lock_guard lock(managerMutex);
        std::vector> result;
        result.reserve(segments.size());
        for(const auto& [name, segment] : segments) {
            result.emplace_back(name, segment);
        }
        return result;
    }
};

class SignalHandler {
private:
    using SignalCallback = std::function<void(int) w-tid="34">;
    std::unordered_map handlers;
    std::mutex handlerMutex;
    
public:
    void registerHandler(int signal, SignalCallback callback) {
        std::lock_guard lock(handlerMutex);
        handlers[signal] = std::move(callback);
    }
    
    void process(int signal) {
        std::lock_guard lock(handlerMutex);
        if(auto it = handlers.find(signal); it != handlers.end()) {
            it->second(signal);
        }
    }

    void clearAllHandlers() {
        std::lock_guard lock(handlerMutex);
        handlers.clear();
    }

    ~SignalHandler() {
        clearAllHandlers();
    }
};

class IPCManager {
private:
    static IPCManager* instance;
    static std::mutex instanceMutex;
    
    MessageQueue messageQueue;
    SharedMemoryManager sharedMem;
    SignalHandler signalHandler;
    
    IPCManager() = default;
    
public:
    static IPCManager& getInstance() {
        std::lock_guard lock(instanceMutex);
        if(!instance) {
            instance = new IPCManager();
        }
        return *instance;
    }
    
    void sendMessage(ProcessId dest, const Message& msg) {
        messageQueue.optimizedSend(dest, msg);
    }
    
    Message receiveMessage(ProcessId receiver) {
        return messageQueue.receive(receiver);
    }
    
    void* createSharedMemory(const std::string& name, size_t size) {
        return sharedMem.allocate(name, size);
    }
    
    bool removeSharedMemory(const std::string& name) {
        return sharedMem.deallocate(name);
    }
    
    void registerSignalHandler(int signal, std::function<void(int) w-tid="40"> handler) {
        signalHandler.registerHandler(signal, std::move(handler));
    }
    
    void handleSignal(int signal) {
        signalHandler.process(signal);
    }
    
    ~IPCManager() {
        // Clean up message queues
        while(!messageQueue.empty()) {
            messageQueue.clear();
        }

        // Clean up shared memory segments
        auto segments = sharedMem.getSegments();
        for(const auto& segment : segments) {
            removeSharedMemory(segment.first);
        }

        // Clean up signal handlers
        signalHandler.clearAllHandlers();

        // Clean up singleton instance
        {
            std::lock_guard lock(instanceMutex);
            if(instance) {
                delete instance;
                instance = nullptr;
            }
        }
    }
};

// Initialize static members
IPCManager* IPCManager::instance = nullptr;
std::mutex IPCManager::instanceMutex;

} // namespace Kernel

#endif // IPC_MANAGER_HPP
void(int)>void(int)>