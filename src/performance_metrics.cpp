#include "../core/performance_metrics.hpp"
#include "../loggin/EventLogger.hpp"
#include 
#include 

namespace Kernel {
namespace Performance {

PerformanceMetrics::PerformanceMetrics() 
    : initialized(false) {
}

PerformanceMetrics::~PerformanceMetrics() {
    shutdown();
}

PerformanceMetrics& PerformanceMetrics::getInstance() {
    static PerformanceMetrics instance;
    return instance;
}

bool PerformanceMetrics::initialize() {
    std::lock_guard lock(mutex);
    
    if (initialized) {
        return true;
    }
    
    EventLogger::log("Initializing Performance Metrics...");
    
    try {
        metrics.clear();
        timers.clear();
        thresholds.clear();
        
        initialized = true;
        EventLogger::log("Performance Metrics initialized successfully");
        return true;
    }
    catch (const std::exception& e) {
        EventLogger::log("Error initializing Performance Metrics: " + std::string(e.what()));
        return false;
    }
}

void PerformanceMetrics::shutdown() {
    std::lock_guard lock(mutex);
    
    if (!initialized) {
        return;
    }
    
    EventLogger::log("Shutting down Performance Metrics...");
    metrics.clear();
    timers.clear();
    thresholds.clear();
    initialized = false;
}

void PerformanceMetrics::recordMetric(const std::string& name, double value, const std::string& unit) {
    if (!initialized) return;
    
    std::lock_guard lock(mutex);
    
    MetricSample sample;
    sample.timestamp = std::chrono::system_clock::now();
    sample.value = value;
    sample.unit = unit;
    
    metrics[name].push_back(sample);
    
    // Check threshold if set
    if (checkThreshold(name)) {
        EventLogger::log("Performance alert: Metric " + name + " exceeded threshold");
    }
}

void PerformanceMetrics::startTimer(const std::string& name) {
    if (!initialized) return;
    
    std::lock_guard lock(mutex);
    timers[name] = std::chrono::system_clock::now();
}

double PerformanceMetrics::stopTimer(const std::string& name) {
    if (!initialized) return 0.0;
    
    std::lock_guard lock(mutex);
    
    auto it = timers.find(name);
    if (it == timers.end()) {
        return 0.0;
    }
    
    auto duration = std::chrono::system_clock::now() - it->second;
    double milliseconds = std::chrono::duration_cast(duration).count();
    
    recordMetric(name, milliseconds, "ms");
    timers.erase(it);
    
    return milliseconds;
}

MetricStats PerformanceMetrics::calculateStats(const std::vector& samples) {
    MetricStats stats = {0.0, 0.0, 0.0, 0.0, samples.size()};
    
    if (samples.empty()) {
        return stats;
    }
    
    // Calculate min, max, and average
    stats.min = samples[0].value;
    stats.max = samples[0].value;
    double sum = 0.0;
    
    for (const auto& sample : samples) {
        stats.min = std::min(stats.min, sample.value);
        stats.max = std::max(stats.max, sample.value);
        sum += sample.value;
    }
    
    stats.average = sum / samples.size();
    
    // Calculate standard deviation
    double sumSquaredDiff = 0.0;
    for (const auto& sample : samples) {
        double diff = sample.value - stats.average;
        sumSquaredDiff += diff * diff;
    }
    
    stats.standardDeviation = std::sqrt(sumSquaredDiff / samples.size());
    return stats;
}

MetricStats PerformanceMetrics::getMetricStats(const std::string& name) {
    std::lock_guard lock(mutex);
    
    auto it = metrics.find(name);
    if (it == metrics.end()) {
        return MetricStats{0.0, 0.0, 0.0, 0.0, 0};
    }
    
    return calculateStats(it->second);
}

void PerformanceMetrics::setThreshold(const std::string& metric, double threshold) {
    std::lock_guard lock(mutex);
    thresholds[metric] = threshold;
}

bool PerformanceMetrics::checkThreshold(const std::string& metric) {
    auto thresholdIt = thresholds.find(metric);
    if (thresholdIt == thresholds.end()) {
        return false;
    }
    
    auto metricIt = metrics.find(metric);
    if (metricIt == metrics.end() || metricIt->second.empty()) {
        return false;
    }
    
    return metricIt->second.back().value > thresholdIt->second;
}

double PerformanceMetrics::getSystemLoad() {
    // Implementation would depend on platform-specific CPU metrics
    return 0.0; // Placeholder
}

std::map PerformanceMetrics::getAllMetrics() {
    std::lock_guard lock(mutex);
    
    std::map currentMetrics;
    for (const auto& metric : metrics) {
        if (!metric.second.empty()) {
            currentMetrics[metric.first] = metric.second.back().value;
        }
    }
    
    return currentMetrics;
}

} // namespace Performance
} // namespace Kernel