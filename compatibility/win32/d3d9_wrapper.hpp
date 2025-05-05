#pragma once
#include 
#include "../drivers/graphics/graphics_device.hpp"
#include "../multimedia/pipeline_manager.hpp"

namespace Win32Compat {

class Direct3D9Wrapper {
private:
    GraphicsDevice* device;
    PipelineManager* pipeline;

public:
    Direct3D9Wrapper() {
        device = GraphicsDevice::getInstance();
        pipeline = PipelineManager::getInstance();
    }

    HRESULT Direct3DCreate9Ex(UINT SDKVersion, IDirect3D9Ex** ppD3D) {
        *ppD3D = createD3D9Interface();
        return S_OK;
    }

    HRESULT CreateDevice(
        UINT Adapter,
        D3DDEVTYPE DeviceType,
        HWND hFocusWindow,
        DWORD BehaviorFlags,
        D3DPRESENT_PARAMETERS* pPresentationParameters,
        IDirect3DDevice9** ppReturnedDeviceInterface
    ) {
        DeviceParams params;
        params.adapter = Adapter;
        params.deviceType = convertDeviceType(DeviceType);
        params.window = hFocusWindow;
        params.flags = BehaviorFlags;
        
        return device->createD3D9Device(params, ppReturnedDeviceInterface);
    }
};

} // namespace Win32Compat