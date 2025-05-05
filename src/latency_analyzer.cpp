#include "../core/latency_analyzer.hpp"
#include "../loggin/EventLogger.hpp"
#include 
#include 
#include 

namespace Kernel {
namespace Performance {

LatencyAnalyzer::LatencyAnalyzer() 
    : initialized(false) {
}

LatencyAnalyzer::~LatencyAnalyzer() {
    shutdown();
}

LatencyAnalyzer& LatencyAnalyzer::getInstance() {
    static LatencyAnalyzer instance;
    return instance;
}

bool LatencyAnalyzer::initialize() {
    std::lock_guard lock(mutex);
    
    if (initialized) {
        return true;
    }
    
    EventLogger::log("Initializing Latency Analyzer...");
    
    try {
        measurements.clear();
        history.clear();
        thresholds.clear();
        
        initialized = true;
        EventLogger::log("Latency Analyzer initialized successfully");
        return true;
    }
    catch (const std::exception& e) {
        EventLogger::log("Error initializing Latency Analyzer: " + std::string(e.what()));
        return false;
    }
}

void LatencyAnalyzer::shutdown() {
    std::lock_guard lock(mutex);
    
    if (!initialized) {
        return;
    }
    
    EventLogger::log("Shutting down Latency Analyzer...");
    measurements.clear();
    history.clear();
    thresholds.clear();
    initialized = false;
}

std::string LatencyAnalyzer::generateMeasurementKey(const std::string& component, 
                                                   const std::string& operation) {
    return component + "::" + operation;
}

void LatencyAnalyzer::startMeasurement(const std::string& component, 
                                     const std::string& operation) {
    if (!initialized) return;
    
    std::lock_guard lock(mutex);
    std::string key = generateMeasurementKey(component, operation);
    measurements[key] = std::chrono::system_clock::now();
}

double LatencyAnalyzer::endMeasurement(const std::string& component, 
                                     const std::string& operation) {
    if (!initialized) return 0.0;
    
    std::lock_guard lock(mutex);
    std::string key = generateMeasurementKey(component, operation);
    
    auto it = measurements.find(key);
    if (it == measurements.end()) {
        return 0.0;
    }
    
    auto duration = std::chrono::system_clock::now() - it->second;
    double latency = std::chrono::duration_cast(duration).count() / 1000.0;
    
    LatencyMeasurement measurement;
    measurement.timestamp = std::chrono::system_clock::now();
    measurement.latency = latency;
    measurement.component = component;
    measurement.operation = operation;
    
    history[component].push_back(measurement);
    measurements.erase(it);
    
    if (isLatencyExceeded(component)) {
        std::stringstream ss;
        ss << "High latency detected in component " << component 
           << ": " << latency << "ms";
        EventLogger::log(ss.str());
    }
    
    return latency;
}

double LatencyAnalyzer::getAverageLatency(const std::string& component) {
    std::lock_guard lock(mutex);
    
    auto it = history.find(component);
    if (it == history.end() || it->second.empty()) {
        return 0.0;
    }
    
    double sum = 0.0;
    for (const auto& measurement : it->second) {
        sum += measurement.latency;
    }
    
    return sum / it->second.size();
}

double LatencyAnalyzer::getMaxLatency(const std::string& component) {
    std::lock_guard lock(mutex);
    
    auto it = history.find(component);
    if (it == history.end() || it->second.empty()) {
        return 0.0;
    }
    
    double max = 0.0;
    for (const auto& measurement : it->second) {
        max = std::max(max, measurement.latency);
    }
    
    return max;
}

void LatencyAnalyzer::setLatencyThreshold(const std::string& component, double threshold) {
    std::lock_guard lock(mutex);
    thresholds[component] = threshold;
}

bool LatencyAnalyzer::isLatencyExceeded(const std::string& component) {
    auto thresholdIt = thresholds.find(component);
    if (thresholdIt == thresholds.end()) {
        return false;
    }
    
    auto historyIt = history.find(component);
    if (historyIt == history.end() || historyIt->second.empty()) {
        return false;
    }
    
    return historyIt->second.back().latency > thresholdIt->second;
}

double LatencyAnalyzer::getPercentile(const std::string& component, double percentile) {
    std::lock_guard lock(mutex);
    
    auto it = history.find(component);
    if (it == history.end() || it->second.empty()) {
        return 0.0;
    }
    
    std::vector latencies;
    for (const auto& measurement : it->second) {
        latencies.push_back(measurement.latency);
    }
    
    std::sort(latencies.begin(), latencies.end());
    size_t index = static_cast(percentile * (latencies.size() - 1) / 100);
    return latencies[index];
}

void LatencyAnalyzer::generateLatencyReport() {
    std::lock_guard lock(mutex);
    
    EventLogger::log("=== Latency Analysis Report ===");
    
    for (const auto& componentHistory : history) {
        const std::string& component = componentHistory.first;
        
        std::stringstream ss;
        ss << "Component: " << component << "\n"
           << "  Average Latency: " << getAverageLatency(component) << "ms\n"
           << "  Max Latency: " << getMaxLatency(component) << "ms\n"
           << "  95th Percentile: " << getPercentile(component, 95.0) << "ms\n"
           << "  99th Percentile: " << getPercentile(component, 99.0) << "ms";
        
        EventLogger::log(ss.str());
    }
}

void LatencyAnalyzer::cleanupOldMeasurements() {
    // Implement cleanup logic for old measurements if needed
}

} // namespace Performance
} // namespace Kernel