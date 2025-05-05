// graphics_implementation.hpp
#pragma once

namespace Win32Compat {

struct GraphicsConfig {
    bool enableAsyncCompute = true;
    bool enableFramePacing = true;
    uint32_t maxPrerenderedFrames = 2;
    RenderQuality quality = RenderQuality::HIGH;
    
    void optimizeForSims4() {
        enableAsyncCompute = true;
        enableFramePacing = true;
        maxPrerenderedFrames = 2;
        quality = RenderQuality::OPTIMIZED;
        setShaderCacheSize(512 * 1024 * 1024); // 512MB shader cache
    }
};

class GraphicsImplementation {
private:
    GraphicsDevice* device;
    GraphicsConfig config;
    
public:
    // Configuration methods
    void setConfig(const GraphicsConfig& newConfig) {
        config = newConfig;
    }

    // DirectX 11 Support
    HRESULT CreateD3D11Device(
        const DeviceParams& params,
        ID3D11Device** ppDevice
    ) {
        return device->createD3D11Device(params, ppDevice);
    }

    // DirectX 12 Support  
    HRESULT CreateD3D12Device(
        const DeviceParams& params,
        ID3D12Device** ppDevice
    ) {
        return device->createD3D12Device(params, ppDevice);
    }

    // Vulkan Support
    VkResult CreateVulkanDevice(
        const VkDeviceCreateInfo* createInfo,
        VkDevice* pDevice
    ) {
        return device->createVulkanDevice(createInfo, pDevice);
    }

    // OpenGL Support
    HGLRC CreateOpenGLContext(HDC hdc) {
        return device->createOpenGLContext(hdc);
    }

    // Shader Support
    HRESULT CompileShader(
        const ShaderParams& params,
        ID3DBlob** ppShader
    ) {
        return device->compileShader(params, ppShader);
    }

    // Advanced Features
    HRESULT CreateMeshShaderPipeline(
        const MeshShaderParams& params
    ) {
        return device->createMeshShaderPipeline(params);
    }

    HRESULT SetupVariableRateShading(
        const VRSParams& params
    ) {
        return device->setupVariableRateShading(params);
    }

    // Optimization methods
    void optimizeForGaming() {
        config.enableAsyncCompute = true;
        config.enableFramePacing = true;
        config.maxPrerenderedFrames = 2;
        device->setOptimalGamingSettings();
    }

    void enableRayTracing(bool enabled) {
        device->setRayTracingEnabled(enabled);
    }
};

}