#include "../core/latency_analyzer.hpp"
#include "../core/performance_metrics.hpp"
#include "../gtest/gtest.h"
#include "../gmock/gmock.h"
#include 
#include 

namespace Kernel {
namespace Performance {
namespace Test {

class LatencyAnalyzerTest : public ::testing::Test {
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

TEST_F(LatencyAnalyzerTest, BasicMeasurement) {
    analyzer->startMeasurement("TestComponent", "TestOperation");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    double latency = analyzer->endMeasurement("TestComponent", "TestOperation");
    
    EXPECT_GE(latency, 45.0);
    EXPECT_LE(latency, 55.0);
}

TEST_F(LatencyAnalyzerTest, ThresholdDetection) {
    analyzer->setLatencyThreshold("TestComponent", 25.0);
    
    analyzer->startMeasurement("TestComponent", "TestOperation");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    analyzer->endMeasurement("TestComponent", "TestOperation");
    
    EXPECT_TRUE(analyzer->isLatencyExceeded("TestComponent"));
}

TEST_F(LatencyAnalyzerTest, AverageLatency) {
    for(int i = 0; i < 5; i++) {
        analyzer->startMeasurement("TestComponent", "TestOperation");
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        analyzer->endMeasurement("TestComponent", "TestOperation");
    }
    
    double avg = analyzer->getAverageLatency("TestComponent");
    EXPECT_GE(avg, 8.0);
    EXPECT_LE(avg, 12.0);
}

TEST_F(LatencyAnalyzerTest, Percentiles) {
    std::vector delays = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    for(int delay : delays) {
        analyzer->startMeasurement("TestComponent", "TestOperation");
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        analyzer->endMeasurement("TestComponent", "TestOperation");
    }
    
    EXPECT_NEAR(analyzer->getPercentile("TestComponent", 95.0), 95.0, 10.0);
    EXPECT_NEAR(analyzer->getPercentile("TestComponent", 50.0), 55.0, 10.0);
}

TEST_F(LatencyAnalyzerTest, MultipleComponents) {
    analyzer->startMeasurement("Component1", "Operation1");
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    analyzer->endMeasurement("Component1", "Operation1");
    
    analyzer->startMeasurement("Component2", "Operation2");
    std::this_thread::sleep_for(std::chrono::milliseconds(40));
    analyzer->endMeasurement("Component2", "Operation2");
    
    EXPECT_GT(analyzer->getMaxLatency("Component2"), 
              analyzer->getMaxLatency("Component1"));
}

} // namespace Test
} // namespace Performance
} // namespace Kernel