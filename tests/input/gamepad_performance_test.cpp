
#include "../gtest/gtest.h"
#include "../tests/input/gamepad_driver.h"
#include "../core/performance_metrics.h"

class GamepadPerformanceTest : public testing::Test {
protected:
    void SetUp() override {
        driver = std::make_unique();
        metrics = std::make_unique();
    }
    
    std::unique_ptr driver;
    std::unique_ptr metrics;
};

TEST_F(GamepadPerformanceTest, InputLatency) {
    const int SAMPLE_COUNT = 1000;
    std::vector latencies;
    
    for(int i = 0; i < SAMPLE_COUNT; i++) {
        auto start = std::chrono::high_resolution_clock::now();
        driver->pollInput();
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast
            (end - start).count();
        latencies.push_back(duration);
    }
    
    double avgLatency = metrics->calculateMean(latencies);
    double stdDev = metrics->calculateStdDev(latencies);
    
    EXPECT_LT(avgLatency, 500.0); // Less than 0.5ms
    EXPECT_LT(stdDev, 100.0);     // Low variance
}

TEST_F(GamepadPerformanceTest, ButtonResponseTime) {
    const int ITERATIONS = 100;
    std::vector responseTimes;
    
    for(int i = 0; i < ITERATIONS; i++) {
        auto start = std::chrono::steady_clock::now();
        
        // Simulate button press
        driver->injectButtonPress(BUTTON_A);
        while(!driver->isButtonPressed(BUTTON_A)) {
            driver->update();
        }
        
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast
            (end - start).count();
            
        responseTimes.push_back(duration);
        
        // Reset for next iteration
        driver->injectButtonRelease(BUTTON_A);
        driver->update();
    }
    
    double avgResponse = metrics->calculateMean(responseTimes);
    EXPECT_LT(avgResponse, 1000.0); // Less than 1ms
}

TEST_F(GamepadPerformanceTest, AnalogStickPrecision) {
    const int POSITIONS = 360;
    std::vector readings;
    
    // Test full rotation of analog stick
    for(int angle = 0; angle < POSITIONS; angle++) {
        float rad = (angle * M_PI) / 180.0f;
        float expectedX = cos(rad);
        float expectedY = sin(rad);
        
        driver->setAnalogStick(0, expectedX, expectedY);
        driver->update();
        
        Vector2D actual = driver->getAnalogStick(0);
        readings.push_back(actual);
        
        // Verify precision within 0.1% error margin
        EXPECT_NEAR(actual.x, expectedX, 0.001f);
        EXPECT_NEAR(actual.y, expectedY, 0.001f);
    }
}