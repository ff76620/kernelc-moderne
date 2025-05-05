#pragma once
#include 
#include 
#include 
#include 
#include 
#include 

class D3D12Wrapper {
private:
    ComPtr m_device;
    ComPtr m_commandQueue;
    ComPtr m_factory;
    std::vector> m_commandAllocators;
    
    // Advanced Features
    struct RaytracingState {
        ComPtr rtPipelineState;
        ComPtr rtRootSignature;
        D3D12_RAYTRACING_PIPELINE_CONFIG rtConfig;
        bool isHardwareAccelerated;
    } m_rtState;

    struct MeshShaderState {
        ComPtr msRootSignature;
        ComPtr msPipelineState;
        D3D12_MESH_SHADER_PIPELINE_STATE_DESC msDesc;
    } m_msState;

    IDStorageFactory* m_dsFactory;
    IDStorage* m_dsDevice;
    ID3D12CommandQueue* m_dsQueue;

public:
    bool InitializeD3D12() {
        UINT dxgiFactoryFlags = 0;
        #ifdef _DEBUG
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        #endif

        // Create factory and enable tearing support
        CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_factory));
        BOOL allowTearing = FALSE;
        m_factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));

        // Create device with maximum feature level
        D3D_FEATURE_LEVEL featureLevels[] = {
            D3D_FEATURE_LEVEL_12_2,
            D3D_FEATURE_LEVEL_12_1,
            D3D_FEATURE_LEVEL_12_0
        };

        for (auto level : featureLevels) {
            if (SUCCEEDED(D3D12CreateDevice(nullptr, level, IID_PPV_ARGS(&m_device))))
                break;
        }

        // Setup command queue with maximum priority
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_GLOBAL_REALTIME;
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));

        // Enable Ultimate features
        D3D12_FEATURE_DATA_D3D12_OPTIONS7 features = {};
        m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &features, sizeof(features));
        
        // Configure ray tracing
        if (features.RaytracingTier >= D3D12_RAYTRACING_TIER_1_1) {
            InitializeRaytracing();
        }

        // Setup mesh shaders
        if (features.MeshShaderTier >= D3D12_MESH_SHADER_TIER_1) {
            InitializeMeshShaders();
        }

        // Enable Variable Rate Shading
        InitializeVRS();

        // Setup DirectStorage
        InitializeDirectStorage();

        return true;
    }

    ~D3D12Wrapper() {
        if (m_dsQueue) m_dsQueue->Release();
        if (m_dsDevice) m_dsDevice->Release(); 
        if (m_dsFactory) m_dsFactory->Release();
    }

private:
    void InitializeRaytracing() {
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS flags = 
            D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE |
            D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;

        // Create raytracing pipeline state
        D3D12_STATE_OBJECT_DESC rtStateDesc = {};
        rtStateDesc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
        m_device->CreateStateObject(&rtStateDesc, IID_PPV_ARGS(&m_rtState.rtPipelineState));
    }

    void InitializeMeshShaders() {
        D3D12_MESH_SHADER_PIPELINE_STATE_DESC meshDesc = {};
        meshDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        
        // Create mesh shader pipeline state
        m_device->CreateMeshShaderPipelineState(&meshDesc, IID_PPV_ARGS(&m_msState.msPipelineState));
    }

    void InitializeVRS() {
        D3D12_FEATURE_DATA_D3D12_OPTIONS6 options6 = {};
        m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS6, &options6, sizeof(options6));
        
        if (options6.VariableShadingRateTier >= D3D12_VARIABLE_SHADING_RATE_TIER_2) {
            // Configure VRS settings for optimal performance
            D3D12_SHADING_RATE_COMBINER combiners[D3D12_RS_SET_SHADING_RATE_COMBINER_COUNT] = {
                D3D12_SHADING_RATE_COMBINER_SUM,
                D3D12_SHADING_RATE_COMBINER_MAX
            };
            m_commandQueue->SetShadingRateImage(nullptr);
            m_commandQueue->SetShadingRate(D3D12_SHADING_RATE_1X1, combiners);
        }
    }

    void InitializeDirectStorage() {
        D3D12_FEATURE_DATA_D3D12_OPTIONS7 options7 = {};
        m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &options7, sizeof(options7));
        
        if (options7.DirectStorageSupported) {
            // Create DirectStorage factory
            DStorageCreateFactory(IID_PPV_ARGS(&m_dsFactory));

            // Configure optimal factory settings
            DSTORAGE_CONFIGURATION config = {};
            config.DisableBypassIO = FALSE;
            config.MaxOperationsPerIOThread = 128;
            config.MaxInstallIOThreads = 8;
            config.MaxHardwareIOThreads = 8;
            config.MaxSoftwareIOThreads = 8;

            m_dsFactory->SetConfiguration(&config);

            // Create DirectStorage device with max compression
            DSTORAGE_DEVICE_DESC deviceDesc = {};
            deviceDesc.Capacity = DSTORAGE_MAX_QUEUE_CAPACITY; 
            deviceDesc.MaxTransferSizeInBytes = 1024 * 1024 * 256; // 256MB
            deviceDesc.MaxImplementationQueueCapacity = DSTORAGE_MAX_QUEUE_CAPACITY;
            deviceDesc.CompressionFormat = DSTORAGE_COMPRESSION_FORMAT_GDEFLATE;
            
            m_dsFactory->CreateDevice(IID_PPV_ARGS(&m_dsDevice));
            m_dsDevice->SetConfiguration(&deviceDesc);

            // Create optimized command queue for DirectStorage
            D3D12_COMMAND_QUEUE_DESC queueDesc = {};
            queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
            queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_GLOBAL_REALTIME;
            queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_dsQueue));

            // Configure memory pools for streaming
            DSTORAGE_MEMORY_POOL_DESC poolDesc = {};
            poolDesc.Flags = DSTORAGE_MEMORY_POOL_FLAG_NONE;
            poolDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
            poolDesc.MinCapacity = 256 * 1024 * 1024; // 256MB minimum
            poolDesc.MaxCapacity = 2048 * 1024 * 1024; // 2GB maximum 
            m_dsDevice->CreateMemoryPool(&poolDesc);

            // Enable compression for maximum throughput
            DSTORAGE_COMPRESSION_DESC compDesc = {};
            compDesc.Format = DSTORAGE_COMPRESSION_FORMAT_GDEFLATE;
            compDesc.BlockSize = DSTORAGE_MAX_BLOCK_SIZE;
            compDesc.EnableHardwareAcceleration = TRUE;
            m_dsDevice->SetCompressionSettings(&compDesc);
        }
    }
};