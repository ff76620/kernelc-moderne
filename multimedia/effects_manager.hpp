#ifndef EFFECTS_MANAGER_HPP
#define EFFECTS_MANAGER_HPP

#include 
#include 
#include 
#include "../include/types.hpp"

namespace Kernel {
namespace Multimedia {

class EffectsManager {
public:
    enum class EffectType {
        AudioFilter,
        AudioEffect,
        VideoFilter,
        VideoEffect,
        ColorCorrection,
        Transition
    };

    struct EffectParameters {
        std::string name;
        std::map parameters;
        bool enabled;
        uint32_t priority;
        double intensity;
        double mixLevel;
    };

    struct EffectChain {
        std::string id;
        std::vector effects;
        bool active;
        double wetDryMix;
        uint32_t processingQuality;
    };

    EffectsManager();
    ~EffectsManager();

    // Initialization and setup
    bool initialize();
    void shutdown();

    // Effect chain management
    bool createEffectChain(const std::string& chainId);
    bool deleteEffectChain(const std::string& chainId);
    bool addEffect(const std::string& chainId, const EffectParameters& effect);
    bool removeEffect(const std::string& chainId, const std::string& effectName);

    // Effect processing
    bool processAudioEffect(const std::string& chainId, 
                          std::vector& audioData,
                          uint32_t sampleRate,
                          uint32_t channels);
    
    bool processVideoEffect(const std::string& chainId,
                          std::vector& videoFrame,
                          uint32_t width,
                          uint32_t height,
                          uint32_t format);

    // Real-time control
    void setEffectParameter(const std::string& chainId,
                          const std::string& effectName,
                          const std::string& parameter,
                          double value);

    void setChainWetDryMix(const std::string& chainId, double mix);
    void setEffectIntensity(const std::string& chainId,
                           const std::string& effectName,
                           double intensity);

    // Performance optimization
    void setProcessingQuality(const std::string& chainId, uint32_t quality);
    void optimizePerformance(uint32_t targetLatencyMs);
    void balanceProcessingLoad();

    // State management
    bool enableEffect(const std::string& chainId, const std::string& effectName);
    bool disableEffect(const std::string& chainId, const std::string& effectName);
    bool isEffectEnabled(const std::string& chainId, const std::string& effectName) const;

    // Monitoring
    struct PerformanceStats {
        double processingTime;
        double cpuUsage;
        uint32_t activeEffects;
        uint32_t bufferSize;
        double latency;
    };

    PerformanceStats getPerformanceStats() const;
    double getChainLatency(const std::string& chainId) const;

private:
    std::map> effectChains;
    PerformanceStats stats;
    uint32_t maxProcessingQuality;
    double targetLatency;

    // Internal processing methods
    bool validateEffectParameters(const EffectParameters& params);
    void updatePerformanceStats(double processingTime);
    void optimizeEffectChain(EffectChain& chain);
    bool applyEffect(const EffectParameters& effect, void* data, uint32_t size);
    
    // Resource management
    void cleanupResources();
    void preloadEffects();
    void cacheProcessingResults();
};

} // namespace Multimedia
} // namespace Kernel

#endif