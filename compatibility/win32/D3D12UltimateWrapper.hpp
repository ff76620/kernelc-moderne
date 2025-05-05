class D3D12UltimateWrapper {
private:
    Microsoft::WRL::ComPtr<ID3D12Device6> m_device;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
    Microsoft::WRL::ComPtr<IDXGIFactory7> m_factory;
    std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>> m_commandAllocators;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList5> m_commandList;

    // Advanced Features State
    struct RaytracingState {
        Microsoft::WRL::ComPtr<ID3D12StateObject> rtPipelineState;
        Microsoft::WRL::ComPtr<ID3D12RootSignature> rtRootSignature;
        D3D12_RAYTRACING_PIPELINE_CONFIG rtConfig;
        bool isHardwareAccelerated;
    } m_rtState;

    struct MeshShaderState {
        Microsoft::WRL::ComPtr<ID3D12RootSignature> msRootSignature;
        Microsoft::WRL::ComPtr<ID3D12PipelineState> msPipelineState;
        D3D12_MESH_SHADER_PIPELINE_STATE_DESC msDesc;
    } m_msState;

    // DirectStorage Support
    IDStorageFactory* m_dsFactory = nullptr;
    IDStorage* m_dsDevice = nullptr;
    ID3D12CommandQueue* m_dsQueue = nullptr;

public:
    bool InitializeD3D12() {
        UINT dxgiFactoryFlags = 0;
        #ifdef _DEBUG
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        #endif

        CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_factory));
        
        BOOL allowTearing = FALSE;
        m_factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));

        D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_12_2,
            D3D_FEATURE_LEVEL_12_1,
            D3D_FEATURE_LEVEL_12_0
        };

        for (auto level : featureLevels) {
            if (SUCCEEDED(D3D12CreateDevice(nullptr, level, IID_PPV_ARGS(&m_device))))
                break;
        }

        // Initialize core features
        InitializeCommandQueue();
        InitializeRayTracing();
        SetupMeshShaders();
        InitializeVRS();
        InitializeDirectStorage();

        return true;
    }

    void InitializeRayTracing() {
        D3D12_FEATURE_DATA_D3D12_OPTIONS5 features = {};
        if (SUCCEEDED(m_device->CheckFeatureSupport(
            D3D12_FEATURE_D3D12_OPTIONS5,
            &features,
            sizeof(features)))) {
            
            if (features.RaytracingTier >= D3D12_RAYTRACING_TIER_1_1) {
                D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS flags = 
                    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE |
                    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;

                D3D12_STATE_OBJECT_DESC rtStateDesc = {};
                rtStateDesc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
                m_device->CreateStateObject(&rtStateDesc, IID_PPV_ARGS(&m_rtState.rtPipelineState));
                
                m_rtState.isHardwareAccelerated = true;
            }
        }
    }

    void SetupMeshShaders() {
        D3D12_FEATURE_DATA_D3D12_OPTIONS7 features = {};
        if (SUCCEEDED(m_device->CheckFeatureSupport(
            D3D12_FEATURE_D3D12_OPTIONS7,
            &features,
            sizeof(features)))) {
            
            if (features.MeshShaderTier >= D3D12_MESH_SHADER_TIER_1) {
                m_msState.msDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
                m_device->CreateMeshShaderPipelineState(&m_msState.msDesc, 
                    IID_PPV_ARGS(&m_msState.msPipelineState));
            }
        }
    }

    ~D3D12UltimateWrapper() {
        if (m_dsQueue) m_dsQueue->Release();
        if (m_dsDevice) m_dsDevice->Release();
        if (m_dsFactory) m_dsFactory->Release();
    }
};