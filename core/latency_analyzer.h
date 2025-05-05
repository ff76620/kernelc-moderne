#ifndef LATENCY_ANALYZER_HPP
#define LATENCY_ANALYZER_HPP

#include "../core/performance_metrics.hpp"
#include "../include/types.hpp"
#include 
#include 
#include 
#include 
#include 

namespace Kernel {
namespace Performance {

struct LatencyMeasurement {
    std::chrono::system_clock::time_point timestamp;
    double latency;
    std::string component;
    std::string operation;
};

class LatencyAnalyzer {
public:
    static LatencyAnalyzer& getInstance();
    
    bool initialize();
    void shutdown();
    
    // Latency measurement
    void startMeasurement(const std::string& component, const std::string& operation);
    double endMeasurement(const std::string& component, const std::string& operation);
    
    // Analysis
    double getAverageLatency(const std::string& component);
    double getMaxLatency(const std::string& component);
    double getPercentile(const std::string& component, double percentile);
    
    // Monitoring
    void setLatencyThreshold(const std::string& component, double threshold);
    bool isLatencyExceeded(const std::string& component);
    
    // Statistics
    std::map> getLatencyHistory();
    void generateLatencyReport();

private:
    LatencyAnalyzer();
    ~LatencyAnalyzer();
    
    std::mutex mutex;
    bool initialized;
    
    std::map measurements;
    std::map> history;
    std::map thresholds;
    
    // Internal methods
    void cleanupOldMeasurements();
    std::string generateMeasurementKey(const std::string& component, const std::string& operation);
    
    // Prevent copying
    LatencyAnalyzer(const LatencyAnalyzer&) = delete;
    LatencyAnalyzer& operator=(const LatencyAnalyzer&) = delete;
};

} // namespace Performance
} // namespace Kernel

#endif