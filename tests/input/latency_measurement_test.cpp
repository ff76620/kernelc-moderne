
#include "../gtest/gtest.h"
#include "../tests/input/gamepad_driver.h"
#include "../core/latency_analyzer.h"

class LatencyMeasurementTest : public testing::Test {
protected:
    void SetUp() override {
        driver = std::make_unique();
        analyzer = std::make_unique();
    }
    
    std::unique_ptr driver;
    std::unique_ptr analyzer;
};

TEST_F(LatencyMeasurementTest, ButtonResponseTime) {
    const int SAMPLE_SIZE = 1000;
    std::vector latencyMeasurements;
    
    for(int i = 0; i < SAMPLE_SIZE; i++) {
        auto start = std::chrono::high_resolution_clock::now();
        
        // Simulate button press
        driver->simulateButtonPress(Button::A);
        
        // Wait for input detection
        while(!driver->isButtonPressed(Button::A)) {
            driver->update();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto latency = std::chrono::duration_cast(end - start);
        latencyMeasurements.push_back(latency.count());
        
        // Reset button state
        driver->simulateButtonRelease(Button::A);
        driver->update();
    }
    
    // Statistical analysis
    double avgLatency = analyzer->calculateMean(latencyMeasurements);
    double stdDev = analyzer->calculateStdDev(latencyMeasurements);
    
    EXPECT_LT(avgLatency, 16000.0); // Less than 16ms
    EXPECT_LT(stdDev, 2000.0);      // Low variance
}

TEST_F(LatencyMeasurementTest, AnalogStickPrecision) {
    const int TEST_POSITIONS = 36; // Test every 10 degrees
    std::vector measurements;
    
    for(int i = 0; i < TEST_POSITIONS; i++) {
        float angle = (2 * M_PI * i) / TEST_POSITIONS;
        Vector2D expected(cos(angle), sin(angle));
        
        driver->simulateStickPosition(Stick::LEFT, expected);
        driver->update();
        
        Vector2D measured = driver->getStickPosition(Stick::LEFT);
        measurements.push_back(measured);
        
        // Verify position accuracy
        EXPECT_NEAR(measured.x, expected.x, 0.01f);
        EXPECT_NEAR(measured.y, expected.y, 0.01f);
        
        // Test for unwanted drift
        for(int j = 0; j < 100; j++) {
            driver->update();
            Vector2D current = driver->getStickPosition(Stick::LEFT);
            EXPECT_NEAR(current.x, measured.x, 0.005f);
            EXPECT_NEAR(current.y, measured.y, 0.005f);
        }
    }
}