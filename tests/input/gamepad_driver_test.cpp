
#include "../gtest/gtest.h"
#include "../gmock/gmock.h"
#include "../tests/input/gamepad_driver.h"

using namespace testing;
using namespace Input;

class GamepadDriverTest : public Test {
protected:
    void SetUp() override {
        driver = std::make_unique();
    }
    
    std::unique_ptr driver;
};

TEST_F(GamepadDriverTest, NormalizeAxisReturnsExpectedValues) {
    EXPECT_FLOAT_EQ(driver->normalizeAxis(32767), 0.99997f);
    EXPECT_FLOAT_EQ(driver->normalizeAxis(0), 0.0f);
    EXPECT_FLOAT_EQ(driver->normalizeAxis(-32768), -1.0f);
}

TEST_F(GamepadDriverTest, NormalizeTriggerReturnsExpectedValues) {
    EXPECT_FLOAT_EQ(driver->normalizeTrigger(255), 1.0f);
    EXPECT_FLOAT_EQ(driver->normalizeTrigger(127), 0.498f);
    EXPECT_FLOAT_EQ(driver->normalizeTrigger(0), 0.0f);
}

TEST_F(GamepadDriverTest, WirelessStatusChecksDeviceCapabilities) {
    // Mock device capabilities
    DeviceCapabilities caps;
    caps.features = FEATURE_WIRELESS;
    
    EXPECT_CALL(*driver, queryDeviceCapabilities(_))
        .WillOnce(DoAll(
            SetArgPointee<0>(caps),
            Return(true)
        ));
    
    EXPECT_TRUE(driver->checkWirelessStatus());
}

TEST_F(GamepadDriverTest, BatteryLevelUpdateForWirelessDevice) {
    // Setup wireless device
    driver->wireless = true;
    
    // Mock battery status
    BatteryStatus status;
    status.level = 75;
    
    EXPECT_CALL(*driver, queryBatteryStatus(_))
        .WillOnce(DoAll(
            SetArgPointee<0>(status),
            Return(true)
        ));
    
    driver->updateBatteryLevel();
    EXPECT_EQ(driver->batteryLevel, 75);
}