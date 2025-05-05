#ifndef VIBRATION_CONTROLLER_H
#define VIBRATION_CONTROLLER_H

#include 
#include 
#include 
#include "../include/types.hpp"

class VibrationController {
private:
    float currentIntensity = 0.0f;
    float currentFrequency = 0.0f;
    float targetIntensity = 0.0f;
    float targetFrequency = 0.0f;
    
    std::chrono::steady_clock::time_point lastUpdate;
    const float INTENSITY_RAMP_RATE = 1.0f; // Units per second
    const float FREQUENCY_RAMP_RATE = 50.0f; // Hz per second

    // Vibration motor calibration data
    struct MotorCalibration {
        float minFrequency = 20.0f;
        float maxFrequency = 200.0f;
        float intensityDeadzone = 0.05f;
        float frequencyTolerance = 0.1f;
    } calibration;

    // Helper function to smoothly interpolate between current and target values
    float interpolate(float current, float target, float rampRate, float deltaTime) {
        float diff = target - current;
        float maxChange = rampRate * deltaTime;
        return current + std::clamp(diff, -maxChange, maxChange);
    }

public:
    VibrationController() {
        lastUpdate = std::chrono::steady_clock::now();
    }

    void setIntensity(float intensity) {
        targetIntensity = std::clamp(intensity, 0.0f, 1.0f);
    }

    void setFrequency(float frequency) {
        targetFrequency = std::clamp(frequency, 
                                   calibration.minFrequency,
                                   calibration.maxFrequency);
    }

    float getCurrentIntensity() const {
        return currentIntensity;
    }

    float getCurrentFrequency() const {
        return currentFrequency;
    }

    void update() {
        auto now = std::chrono::steady_clock::now();
        float deltaTime = std::chrono::duration_cast>(
            now - lastUpdate).count();
        lastUpdate = now;

        // Smooth transitions for both intensity and frequency
        currentIntensity = interpolate(currentIntensity, targetIntensity,
                                     INTENSITY_RAMP_RATE, deltaTime);
        currentFrequency = interpolate(currentFrequency, targetFrequency,
                                     FREQUENCY_RAMP_RATE, deltaTime);

        // Apply deadzone to prevent motor jitter at very low intensities
        if (currentIntensity < calibration.intensityDeadzone) {
            currentIntensity = 0.0f;
        }

        // Ensure frequency stays within viable range
        if (currentFrequency < calibration.minFrequency) {
            currentFrequency = calibration.minFrequency;
        }
        if (currentFrequency > calibration.maxFrequency) {
            currentFrequency = calibration.maxFrequency;
        }

        // Here you would typically send the actual commands to the hardware
        // applyToHardware(currentIntensity, currentFrequency);
    }

    // Calibration methods
    void calibrateMotor(float minFreq, float maxFreq, float deadzone) {
        calibration.minFrequency = minFreq;
        calibration.maxFrequency = maxFreq;
        calibration.intensityDeadzone = deadzone;
    }

    void setFrequencyTolerance(float tolerance) {
        calibration.frequencyTolerance = std::clamp(tolerance, 0.01f, 0.5f);
    }

    // Stop all vibration
    void stop() {
        setIntensity(0.0f);
        setFrequency(calibration.minFrequency);
        update(); // Force immediate update
    }

    // Create simple vibration patterns
    void pulse(float intensity, float duration) {
        setIntensity(intensity);
        // Note: In a real implementation, you'd want to handle the duration
        // in an async way or through a pattern system
    }

    void rumble(float intensity, float lowFreq, float highFreq, float mix) {
        float blendedFreq = lowFreq * (1.0f - mix) + highFreq * mix;
        setIntensity(intensity);
        setFrequency(blendedFreq);
    }
};

#endif // VIBRATION_CONTROLLER_H