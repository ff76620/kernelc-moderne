
#pragma once

#include "kernel/drivers/graphics/graphics_manager.hpp"
#include "kernel/compatibility/win32/universal_graphics.hpp"
#include "kernel/core/kernel.hpp"
#include "kernel/drivers/graphics/graphics_device.hpp"

namespace Win32Compat {

struct GraphicsConfig {
    bool enable_universal_apis = true;
    bool enable_ai_upscaling = true;
    bool enable_adaptive_sync = true;
    int shader_cache_size = 2048;  // MB
    int texture_streaming_buffer = 4096;  // MB

    struct PerformanceProfile {
        int target_fps = 144;
        bool dynamic_resolution = true;
        std::string quality_preset = "ULTRA";
        std::string latency_mode = "ULTRA_LOW";
    } performance;
};

class UniversalGraphicsManager {
private:
    static std::unique_ptr instance;
    std::unordered_map deviceContexts;
    std::unique_ptr graphicsDevice;
    
    DXGIWrapper m_dxgi;
    D3D12Wrapper m_d3d12;
    UniversalGraphicsContext graphics;
    AdaptiveRenderingEngine renderer;
    bool m_rayTracingEnabled;

    // Advanced graphics features
    AIAssistedUpscaler upscaler;
    DynamicShaderCompiler shaderCompiler;
    UniversalGraphicsOptimizer optimizer;
    GraphicsConfig config;
    
    // Performance monitoring
    PerformanceMetrics metrics;

public:
    UniversalGraphicsManager();
    static UniversalGraphicsManager* getInstance();
    
    void initializeModernGraphics();
    void optimizeForGaming();
    void initializeUniversalGraphics();
    void setupPerformanceMonitoring();
    void optimizeForGame(const std::string& gameProfile);
    PerformanceMetrics getCurrentMetrics();

private:
    void setupUniversalShaderCompilation();
    void configureAdaptivePerformance();
};

class TextureCacheManager {
private:
    struct CacheEntry {
        void* textureData;
        size_t size;
        uint64_t lastAccess;
    };

    std::unordered_map textureCache;
    const size_t MAX_CACHE_SIZE = 512 * 1024 * 1024;  // 512MB
    size_t currentCacheSize = 0;
    std::mutex cacheMutex;

public:
    void* getCachedTexture(const std::string& texturePath);
    void cacheTexture(const std::string& texturePath, void* data, size_t size);

private:
    uint64_t getCurrentTimestamp();
    void evictOldestTexture();
};

class MemoryManager {
private:
    PageAllocator m_pageAllocator;
    SwapManager m_swapManager;

public:
    void optimizeForGaming();
    void configureGCForGaming();
};

} // namespace Win32Compat
    