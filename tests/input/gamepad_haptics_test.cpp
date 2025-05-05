
#include "../gtest/gtest.h"
#include "../gmock/gmock.h"
#include "../tests/input/gamepad_driver.h"

using namespace testing;
using namespace Input;

class GamepadHapticsTest : public Test {
protected:
    void SetUp() override {
        driver = std::make_unique();
    }
    
    std::unique_ptr driver;
};

TEST_F(GamepadHapticsTest, SetVibrationIntensity) {
    EXPECT_TRUE(driver->setVibration(0.5f, 0.75f));
    
    float leftMotor, rightMotor;
    driver->getVibrationState(&leftMotor, &rightMotor);
    
    EXPECT_FLOAT_EQ(leftMotor, 0.5f);
    EXPECT_FLOAT_EQ(rightMotor, 0.75f);
}

TEST_F(GamepadHapticsTest, VibrationLimits) {
    // Test upper bounds
    EXPECT_TRUE(driver->setVibration(1.0f, 1.0f));
    
    // Test lower bounds
    EXPECT_TRUE(driver->setVibration(0.0f, 0.0f));
    
    // Test invalid values
    EXPECT_FALSE(driver->setVibration(-0.1f, 1.1f));
}

TEST_F(GamepadHapticsTest, ForceFeedbackCapability) {
    DeviceCapabilities caps;
    caps.features = FEATURE_FORCE_FEEDBACK;
    
    EXPECT_CALL(*driver, queryDeviceCapabilities(_))
        .WillOnce(DoAll(
            SetArgPointee<0>(caps),
            Return(true)
        ));
    
    EXPECT_TRUE(driver->supportsForceFeedback());
}

TEST_F(GamepadHapticsTest, ForceFeedbackEffect) {
    ForceFeedbackEffect effect;
    effect.type = EFFECT_SPRING;
    effect.strength = 0.8f;
    effect.duration = 500; // ms
    
    EXPECT_TRUE(driver->playForceFeedbackEffect(effect));
    EXPECT_TRUE(driver->isEffectPlaying());
    
    // Wait for effect completion
    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    EXPECT_FALSE(driver->isEffectPlaying());
}