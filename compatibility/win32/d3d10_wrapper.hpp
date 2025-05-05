#pragma once
#include 
#include "../drivers/graphics/graphics_device.hpp"
#include "../multimedia/pipeline_manager.hpp"

namespace Win32Compat {
class D3D10Wrapper {
private:
    GraphicsDevice* device;
    PipelineManager* pipeline;

public:
    D3D10Wrapper() {
        device = GraphicsDevice::getInstance();
        pipeline = PipelineManager::getInstance();
    }

    HRESULT D3D10CreateDevice(
        IDXGIAdapter *pAdapter,
        D3D10_DRIVER_TYPE DriverType,
        HMODULE Software,
        UINT Flags,
        UINT SDKVersion,
        ID3D10Device **ppDevice
    ) {
        DeviceParams params;
        params.adapter = pAdapter;
        params.driverType = convertDriverType(DriverType);
        params.flags = Flags;
        
        return device->createD3D10Device(params, ppDevice);
    }

    HRESULT D3D10CreateDeviceAndSwapChain(
        IDXGIAdapter *pAdapter,
        D3D10_DRIVER_TYPE DriverType,
        HMODULE Software,
        UINT Flags,
        UINT SDKVersion,
        DXGI_SWAP_CHAIN_DESC *pSwapChainDesc,
        IDXGISwapChain **ppSwapChain,
        ID3D10Device **ppDevice
    ) {
        DeviceParams params;
        params.adapter = pAdapter;
        params.driverType = convertDriverType(DriverType);
        params.flags = Flags;
        params.swapChainDesc = convertSwapChainDesc(pSwapChainDesc);
        
        return device->createD3D10DeviceAndSwapChain(params, ppSwapChain, ppDevice);
    }
};
} // namespace Win32Compat