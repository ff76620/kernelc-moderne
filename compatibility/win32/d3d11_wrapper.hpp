#pragma once
#include 
#include "../drivers/graphics/graphics_device.hpp"
#include "../multimedia/pipeline_manager.hpp"

namespace Win32Compat {

class Direct3D11Wrapper {
private:
    GraphicsDevice* device;
    PipelineManager* pipeline;

public:
    Direct3D11Wrapper() {
        device = GraphicsDevice::getInstance();
        pipeline = PipelineManager::getInstance(); 
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
        DeviceParams params;
        params.adapter = pAdapter;
        params.driverType = convertDriverType(DriverType);
        params.flags = Flags;
        params.featureLevels = convertFeatureLevels(pFeatureLevels, FeatureLevels);

        return device->createD3D11Device(params, ppDevice, ppImmediateContext);
    }
	HRESULT CreateRenderTargetView(
            ID3D11Device* pd3dDevice,
            ID3D11Resource* pResource,
            const D3D11_RENDER_TARGET_VIEW_DESC* pDesc,
            ID3D11RenderTargetView** ppRTView
        ) {
            return device->createRenderTargetView(pd3dDevice, pResource, pDesc, ppRTView);
        }
	.
};

} // namespace Win32Compat