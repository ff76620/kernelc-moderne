#pragma once
#include 
#include "../drivers/graphics/graphics_device.hpp"
#include "../multimedia/pipeline_manager.hpp"

namespace Win32Compat {
class DXGIWrapper {
private:
    GraphicsDevice* device;
    PipelineManager* pipeline;

public:
    DXGIWrapper() {
        device = GraphicsDevice::getInstance();
        pipeline = PipelineManager::getInstance();
    }

    HRESULT CreateDXGIFactory(
        REFIID riid,
        void** ppFactory
    ) {
        return device->createDXGIFactory(riid, ppFactory);
    }

    HRESULT CreateDXGISwapChain(
        IUnknown* pDevice,
        DXGI_SWAP_CHAIN_DESC* pDesc,
        IDXGISwapChain** ppSwapChain
    ) {
        SwapChainParams params;
        params.device = pDevice;
        params.desc = convertSwapChainDesc(pDesc);
        
        return pipeline->createSwapChain(params, ppSwapChain);
    }

    HRESULT EnumAdapters(
        UINT Adapter,
        IDXGIAdapter** ppAdapter
    ) {
        return device->enumAdapters(Adapter, ppAdapter);
    }

    HRESULT GetDesc(
        DXGI_ADAPTER_DESC* pDesc
    ) {
        return device->getAdapterDesc(pDesc);
    }

    HRESULT EnumOutputs(
        UINT Output,
        IDXGIOutput** ppOutput
    ) {
        return device->enumOutputs(Output, ppOutput); 
    }

    HRESULT SetFullscreenState(
        BOOL Fullscreen,
        IDXGIOutput* pTarget
    ) {
        return pipeline->setFullscreenState(Fullscreen, pTarget);
    }

    HRESULT ResizeBuffers(
        UINT BufferCount,
        UINT Width,
        UINT Height,
        DXGI_FORMAT NewFormat,
        UINT SwapChainFlags
    ) {
        return pipeline->resizeSwapChainBuffers(
            BufferCount,
            Width, 
            Height,
            NewFormat,
            SwapChainFlags
        );
    }

    HRESULT Present(
        UINT SyncInterval,
        UINT Flags
    ) {
        return pipeline->presentFrame(SyncInterval, Flags);
    }
};
} // namespace Win32Compat