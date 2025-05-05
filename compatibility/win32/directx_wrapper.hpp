#pragma once
#include 
#include "../drivers/graphics/graphics_manager.hpp"
#include "../multimedia/pipeline_manager.hpp"

namespace Win32Compat {

class DirectXWrapper {
private:
    GraphicsManager* graphicsManager;
    PipelineManager* pipelineManager;

public:
    DirectXWrapper() {
        graphicsManager = GraphicsManager::getInstance();
        pipelineManager = PipelineManager::getInstance();
    }

    HRESULT D3D11CreateDevice(
        IDXGIAdapter* pAdapter,
        D3D_DRIVER_TYPE DriverType,
        HMODULE Software,
        UINT Flags,
        const D3D_FEATURE_LEVEL* pFeatureLevels,
        UINT FeatureLevels,
        UINT SDKVersion,
        ID3D11Device** ppDevice,
        D3D_FEATURE_LEVEL* pFeatureLevel,
        ID3D11DeviceContext** ppImmediateContext
    ) {
        DeviceCreationParams params;
        params.driverType = convertDriverType(DriverType);
        params.featureLevel = selectFeatureLevel(pFeatureLevels, FeatureLevels);
        
        return graphicsManager->createDevice(params, ppDevice, ppImmediateContext);
    }

    HRESULT CreateRenderTargetView(
        ID3D11Device* pDevice,
        ID3D11Resource* pResource,
        const D3D11_RENDER_TARGET_VIEW_DESC* pDesc,
        ID3D11RenderTargetView** ppRTView
    ) {
        RenderTargetParams params;
        params.resource = pResource;
        params.format = convertFormat(pDesc);
        
        return graphicsManager->createRenderTarget(params, ppRTView);
    }

    void RSSetViewports(
        ID3D11DeviceContext* pContext,
        UINT NumViewports,
        const D3D11_VIEWPORT* pViewports
    ) {
        ViewportParams params;
        params.count = NumViewports;
        params.viewports = convertViewports(pViewports);
        
        pipelineManager->setViewports(params);
    }

    HRESULT CreateTexture2D(
        ID3D11Device* pDevice,
        const D3D11_TEXTURE2D_DESC* pDesc,
        const D3D11_SUBRESOURCE_DATA* pInitialData,
        ID3D11Texture2D** ppTexture2D
    ) {
        TextureParams params;
        params.width = pDesc->Width;
        params.height = pDesc->Height;
        params.format = convertFormat(pDesc->Format);
        
        return graphicsManager->createTexture(params, ppTexture2D);
    }
};

} // namespace Win32Compat