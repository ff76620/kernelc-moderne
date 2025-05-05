#include "../core/latency_analyzer.hpp"
#include "../gtest/gtest.h"
#include 
#include 
#include 

namespace Kernel {
namespace Performance {
namespace Test {

class LatencyAnalyzerPerformanceTest : public ::testing::Test {
protected:
    void SetUp() override {
        analyzer = &LatencyAnalyzer::getInstance();
        analyzer->initialize();
    }

    void TearDown() override {
        analyzer->shutdown();
    }

    LatencyAnalyzer* analyzer;
};

TEST_F(LatencyAnalyzerPerformanceTest, ConcurrentMeasurements) {
    const int NUM_THREADS = 10;
    const int MEASUREMENTS_PER_THREAD = 1000;
    
    std::vector> futures;
    
    auto measurementTask = [this](int threadId) {
        std::string component = "Component" + std::to_string(threadId);
        
        for(int i = 0; i < MEASUREMENTS_PER_THREAD; i++) {
            analyzer->startMeasurement(component, "StressOperation");
            std::this_thread::sleep_for(std::chrono::microseconds(100));
            double latency = analyzer->endMeasurement(component, "StressOperation");
            EXPECT_GT(latency, 0.0);
        }
    };

    // Launch concurrent measurement tasks
    for(int i = 0; i < NUM_THREADS; i++) {
        futures.push_back(std::async(std::launch::async, measurementTask, i));
    }

    // Wait for all tasks to complete
    for(auto& future : futures) {
        future.wait();
    }
}

TEST_F(LatencyAnalyzerPerformanceTest, MemoryStressTest) {
    const int NUM_COMPONENTS = 100;
    const int MEASUREMENTS_PER_COMPONENT = 1000;
    
    for(int i = 0; i < NUM_COMPONENTS; i++) {
        std::string component = "StressComponent" + std::to_string(i);
        
        for(int j = 0; j < MEASUREMENTS_PER_COMPONENT; j++) {
            analyzer->startMeasurement(component, "StressOperation");
            std::this_thread::sleep_for(std::chrono::microseconds(50));
            analyzer->endMeasurement(component, "StressOperation");
        }
        
        // Verify statistics are available
        EXPECT_GT(analyzer->getAverageLatency(component), 0.0);
        EXPECT_GT(analyzer->getMaxLatency(component), 0.0);
        EXPECT_GT(analyzer->getPercentile(component, 95.0), 0.0);
    }
}

TEST_F(LatencyAnalyzerPerformanceTest, ThresholdPerformance) {
    const int NUM_COMPONENTS = 50;
    const int CHECKS_PER_COMPONENT = 1000;
    
    // Set thresholds for all components
    for(int i = 0; i < NUM_COMPONENTS; i++) {
        std::string component = "ThresholdComponent" + std::to_string(i);
        analyzer->setLatencyThreshold(component, 1.0);
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Perform rapid threshold checks
    for(int i = 0; i < NUM_COMPONENTS; i++) {
        std::string component = "ThresholdComponent" + std::to_string(i);
        
        for(int j = 0; j < CHECKS_PER_COMPONENT; j++) {
            analyzer->startMeasurement(component, "Operation");
            std::this_thread::sleep_for(std::chrono::microseconds(10));
            analyzer->endMeasurement(component, "Operation");
            bool exceeded = analyzer->isLatencyExceeded(component);
            EXPECT_FALSE(exceeded);
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast(endTime - startTime);
    
    // Test should complete within reasonable time
    EXPECT_LT(duration.count(), 30);
}

TEST_F(LatencyAnalyzerPerformanceTest, ReportGenerationPerformance) {
    const int NUM_COMPONENTS = 25;
    const int MEASUREMENTS_PER_COMPONENT = 500;
    
    // Generate substantial measurement data
    for(int i = 0; i < NUM_COMPONENTS; i++) {
        std::string component = "ReportComponent" + std::to_string(i);
        
        for(int j = 0; j < MEASUREMENTS_PER_COMPONENT; j++) {
            analyzer->startMeasurement(component, "Operation");
            std::this_thread::sleep_for(std::chrono::microseconds(random() % 200));
            analyzer->endMeasurement(component, "Operation");
        }
    }
    
    auto startTime = std::chrono::high_resolution_clock::now();
    analyzer->generateLatencyReport();
    auto endTime = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast(endTime - startTime);
    
    // Report generation should be reasonably fast
    EXPECT_LT(duration.count(), 1000);
}

} // namespace Test
} // namespace Performance
} // namespace Kernel