
#pragma once

#include "../core/kernel.hpp"
#include "../drivers/graphics/graphics_device.hpp"
#include "../security/security_manager.hpp"

namespace Win32Compat {

// Configuration optimisée pour Les Sims 4
#define SIMS4_TEXTURE_CACHE_SIZE (512 * 1024 * 1024)  // 512 MB cache
#define SIMS4_MESH_CACHE_SIZE (128 * 1024 * 1024)    // 128 MB mesh cache
#define SIMS4_SHADER_CACHE_ENABLED true
#define SIMS4_ASYNC_LOADING true
#define SIMS4_MEMORY_POOL_SIZE (2048 * 1024 * 1024)  // 2 GB pool
#define SIMS4_THREAD_PRIORITY HIGH
#define SIMS4_IO_PRIORITY HIGH
#define SIMS4_GPU_OPTIMIZATION ENABLED
#define SIMS4_MEMORY_PRIORITY HIGH

class GameCompatibility {
private:
    GraphicsDevice* device;
    SecurityManager* security;
    FileCache fileCache;
    ProcessManager processManager;
    MemoryManager memoryManager;
    
    struct Sims4Profile {
        static void optimizeTextureSettings() {
            setTextureCompressionLevel(HIGH);
            setLODBias(0.8f);
            enableMipMapStreaming();
        }
        
        static void preloadCommonAssets() {
            preloadCharacterBaseMeshes();
            preloadEnvironmentTextures();
        }
    };
    
public:
    // Configuration améliorée pour Les Sims 4
    void configureSims4() {
        // Paramètres graphiques de base
        setDX11Mode(OPTIMIZED);
        allocateTextureCache(SIMS4_TEXTURE_CACHE_SIZE);
        allocateMeshCache(SIMS4_MESH_CACHE_SIZE);
        
        // Optimisations spécifiques Sims 4
        Sims4Profile::optimizeTextureSettings();
        Sims4Profile::preloadCommonAssets();
        setSimulationThreads(6);
        
        // Optimisations modernes
        device->initModernGameSupport({
            .shaderCache = 2048,
            .asyncCompute = true,
            .rayTracing = true,
            .textureCompression = HIGH,
            .asyncLoading = true
        });

        // Configuration du cache et des priorités
        fileCache.setAssetPriority("sims4", Priority::HIGH);
        fileCache.optimizeForGame("SIMS4");
        processManager.enableMultiThreading(true);
        processManager.optimizeForGame("SIMS4");

        // Gestion mémoire optimisée
        memoryManager.optimizeForGaming();
        memoryManager.configureLowLatency();
        enableLargePageSupport();
    }

    // Configuration pour jeux modernes
    void configureModernGames() {
        enableDXR();
        enableDLSS();
        enableFSR2();
        setShaderCache(2048MB);
        
        device->enableAdvancedFeatures({
            .dlss = true,
            .rayTracing = true,
            .variableRateShading = true,
            .meshShaders = true,
            .asyncCompute = true
        });

        // Optimisations supplémentaires
        fileCache.setAssetPriority("modern", Priority::HIGH);
        processManager.enableMultiThreading(true);
        memoryManager.optimizeForGaming();
    }

    // Support des jeux modernes
    HRESULT InitializeGameSupport(const GameParams& params) {
        enableAsyncCompute();
        setTextureStreamingBuffer(1024MB);
        enableAdaptiveSync();
        return device->initModernGameSupport(params);
    }

    // Support DRM
    HRESULT ValidateDRM(const DRMParams& params) {
        return security->validateDRM(params);
    }

    // Support Shader Model
    HRESULT InitializeShaderModel(const ShaderModelParams& params) {
        return device->initShaderModel(params);
    }

    // Fonctionnalités graphiques avancées
    HRESULT EnableAdvancedFeatures(const FeatureParams& params) {
        return device->enableAdvancedFeatures(params);
    }

    // Gestion mémoire optimisée
    void optimizeMemory() {
        allocateGameCache(2048MB);
        setMemoryPriority(PRIORITY_GAMING);
        enableLargePageSupport();
        memoryManager.optimizeForGaming();
        
        // Nouveau système de cache GPU
        GraphicsResourceManager::getInstance()->configure({
            .textureCache = SIMS4_TEXTURE_CACHE_SIZE,
            .meshCache = SIMS4_MESH_CACHE_SIZE,
            .priorityMode = CachePriority::GAMING
        });
    }

    // Support hérité
    void configureLegacySupport() {
        enableDX9Compatibility();
        setLegacyTextureFormat(TRUE);
        enableLegacyShaderModel();
        processManager.enableMultiThreading(false);
        
        // Support des anciennes APIs
        device->enableLegacyFeatures({
            .dx9Support = true,
            .openglSupport = true,
            .vulkanSupport = false
        });
    }

    // Gestion des performances
    void configurePerformanceProfile(const PerformanceProfile& profile) {
        switch(profile.type) {
            case PERFORMANCE_GAMING:
                setGPUPowerState(GPU_PERFORMANCE);
                setCPUPowerState(CPU_PERFORMANCE);
                break;
            case PERFORMANCE_BALANCED:
                setGPUPowerState(GPU_BALANCED);
                setCPUPowerState(CPU_BALANCED);
                break;
            case PERFORMANCE_POWERSAVE:
                setGPUPowerState(GPU_POWERSAVE);
                setCPUPowerState(CPU_POWERSAVE);
                break;
        }
    }

    // API DirectX moderne
    void initializeModernDirectX() {
        device->initializeAPI({
            .d3d12 = true,
            .raytracing = true,
            .meshShaders = true,
            .variableRateShading = true
        });
    }

    // Gestion du multithread
    void configureThreading(const ThreadingConfig& config) {
        processManager.setThreadCount(config.threadCount);
        processManager.setThreadPriority(config.priority);
        processManager.enableThreadPinning(config.pinThreads);
    }

    // Gestion des ressources
    void configureResourceManagement() {
        GraphicsResourceManager::getInstance()->setResourcePolicy({
            .dynamicLoading = true,
            .streamingEnabled = true,
            .resourcePriority = PRIORITY_HIGH
        });
    }
};

} // namespace Win32Compat