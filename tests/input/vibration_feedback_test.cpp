
#include "../gtest/gtest.h"
#include "../tests/input/gamepad_driver.h"
#include "../haptics/vibration_controller.h"

class VibrationFeedbackTest : public testing::Test {
protected:
    void SetUp() override {
        driver = std::make_unique();
        vibration = std::make_unique();
    }
    
    std::unique_ptr driver;
    std::unique_ptr vibration;
};

TEST_F(VibrationFeedbackTest, IntensityControl) {
    const int INTENSITY_LEVELS = 10;
    std::vector measuredIntensities;
    
    for(int i = 0; i < INTENSITY_LEVELS; i++) {
        float targetIntensity = i / (float)(INTENSITY_LEVELS - 1);
        
        vibration->setIntensity(targetIntensity);
        driver->update();
        
        float measured = vibration->getCurrentIntensity();
        measuredIntensities.push_back(measured);
        
        // Verify intensity accuracy
        EXPECT_NEAR(measured, targetIntensity, 0.05f);
        
        // Test intensity stability
        for(int j = 0; j < 50; j++) {
            driver->update();
            float current = vibration->getCurrentIntensity();
            EXPECT_NEAR(current, measured, 0.02f);
        }
    }
}

TEST_F(VibrationFeedbackTest, FrequencyResponse) {
    const std::vector TEST_FREQUENCIES = {
        20.0f,  // Low rumble
        50.0f,  // Medium vibration
        100.0f, // High frequency
        200.0f  // Ultra feedback
    };
    
    for(float freq : TEST_FREQUENCIES) {
        vibration->setFrequency(freq);
        vibration->setIntensity(1.0f);
        driver->update();
        
        // Measure actual frequency
        float measured = vibration->getCurrentFrequency();
        
        // Verify frequency accuracy
        EXPECT_NEAR(measured, freq, freq * 0.1f); // 10% tolerance
        
        // Test frequency stability
        for(int i = 0; i < 100; i++) {
            driver->update();
            float current = vibration->getCurrentFrequency();
            EXPECT_NEAR(current, measured, measured * 0.05f);
        }
    }
}