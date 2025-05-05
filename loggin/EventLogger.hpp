#ifndef EVENT_LOGGER_HPP
#define EVENT_LOGGER_HPP

#include 
#include 
#include "../include/types.hpp"

namespace Kernel {
namespace Logging {

class EventLogger {
public:
    EventLogger();
    ~EventLogger();

    void initialize();
    void shutdown();

    // Logging Operations
    void logEvent(const Event& event);
    void logError(const std::string& message);
    void logWarning(const std::string& message);
    void logDebug(const std::string& message);
    
    // Log Management
    void flush();
    void rotate();
    void compress();
    
    // Log Analysis
    struct LogStats {
        uint32_t totalEvents;
        uint32_t errorCount;
        uint32_t warningCount;
        std::vector recentEntries;
    };

    LogStats getStats() const;
    void analyzeLogs();
    void exportLogs(const std::string& format);

private:
    std::queue eventQueue;
    LogConfiguration config;
    
    void writeToFile();
    void cleanOldLogs();
    void validateEntry(const LogEntry& entry);
};

} // namespace Logging
} // namespace Kernel

#endif