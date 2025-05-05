
TEST_F(GamepadPerformanceTest, PowerConsumption) {
    const int MEASUREMENT_PERIOD = 1000; // 1 second
    std::vector powerReadings;
    
    // Measure baseline power consumption
    float baselinePower = driver->measurePowerConsumption();
    
    // Test different operation modes
    std::vector modes = {
        OperationMode::IDLE,
        OperationMode::ACTIVE,
        OperationMode::VIBRATION,
        OperationMode::FULL_LOAD
    };
    
    for(auto mode : modes) {
        driver->setOperationMode(mode);
        
        // Collect power samples
        for(int i = 0; i < 100; i++) {
            driver->update();
            float power = driver->measurePowerConsumption();
            powerReadings.push_back(power);
            
            // Verify power consumption limits
            switch(mode) {
                case OperationMode::IDLE:
                    EXPECT_LT(power, baselinePower * 1.2f);
                    break;
                case OperationMode::ACTIVE:
                    EXPECT_LT(power, baselinePower * 2.0f);
                    break;
                case OperationMode::VIBRATION:
                    EXPECT_LT(power, baselinePower * 3.0f);
                    break;
                case OperationMode::FULL_LOAD:
                    EXPECT_LT(power, baselinePower * 4.0f);
                    break;
            }
        }
    }
}