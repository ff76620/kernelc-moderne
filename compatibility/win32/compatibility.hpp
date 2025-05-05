
            #pragma once

            namespace Win32Compat {

            template
            class Singleton {
            protected:
                static std::unique_ptr instance;
                
            public:
                static std::shared_ptr getInstance();
            };

            class WindowsCompatibilityLayer : public Singleton {
            private:
                // Core API wrappers and modules
                HMODULE kernel32;
                HMODULE user32;
                HMODULE gdi32;
                HMODULE d3d11;
                HMODULE d3d12;
                HMODULE dxgi;
                std::shared_ptr advApi32;
                std::shared_ptr comCtl32;
                std::shared_ptr comDlg32;
                
                std::unordered_map cachedFunctions;
                ID3D11Device* m_device;
                ID3D11DeviceContext* m_context; 
                IDXGISwapChain* m_swapChain;
                
                SystemCallTranslator syscallTranslator;
                WinAPIEmulator apiEmulator;
                VirtualizationEngine virtEngine;
                SystemVersionChecker versionChecker;
                APICompatibilityLayer apiLayer;

                template
                class HandleWrapper {
                private:
                    H handle;
                public:
                    HandleWrapper(H h);
                    ~HandleWrapper();
                    operator H() const;
                };

            public:
                struct InitializeParams {
                    std::optional appName;
                    std::optional flags; 
                    std::vector requiredAPIs;
                };

                WindowsCompatibilityLayer() : 
                    kernel32(NULL), user32(NULL), gdi32(NULL),
                    d3d11(NULL), d3d12(NULL), dxgi(NULL),
                    m_device(nullptr), m_context(nullptr), m_swapChain(nullptr) {}

                bool InitializeWrappers() {
                    kernel32 = LoadLibrary("kernel32.dll");
                    user32 = LoadLibrary("user32.dll");
                    gdi32 = LoadLibrary("gdi32.dll");
                    d3d11 = LoadLibrary("d3d11.dll");
                    d3d12 = LoadLibrary("d3d12.dll");
                    dxgi = LoadLibrary("dxgi.dll");
                    
                    return (kernel32 && user32 && gdi32 && d3d11 && d3d12 && dxgi);
                }

                template
                T GetProcAddress(const char* moduleName, const char* functionName) {
                    std::string key = std::string(moduleName) + "::" + functionName;
                    
                    if (cachedFunctions.find(key) == cachedFunctions.end()) {
                        HMODULE module = NULL;
                        if (strcmp(moduleName, "kernel32") == 0) module = kernel32;
                        else if (strcmp(moduleName, "user32") == 0) module = user32;
                        else if (strcmp(moduleName, "gdi32") == 0) module = gdi32;
                        else if (strcmp(moduleName, "d3d11") == 0) module = d3d11;
                        else if (strcmp(moduleName, "d3d12") == 0) module = d3d12;
                        else if (strcmp(moduleName, "dxgi") == 0) module = dxgi;
                        
                        if (module) {
                            cachedFunctions[key] = ::GetProcAddress(module, functionName);
                        }
                    }
                    
                    return reinterpret_cast(cachedFunctions[key]);
                }

                void initializeWinAPI() {
                    apiEmulator.initialize();
                    syscallTranslator.registerHandlers();
                    virtEngine.setupEnvironment();
                }

                void registerSystemCalls() {
                    syscallTranslator.mapSystemCalls();
                    apiEmulator.registerAPIs();
                }

                void setupVirtualization() {
                    virtEngine.configureHypervisor();
                    virtEngine.initializeVirtualDevices(); 
                }

                [[nodiscard]] std::expected initialize(const InitializeParams& params);
                [[nodiscard]] std::expected createDeviceContext();

                bool InitializeGraphics() {
                    // Initialize DirectX components
                    auto D3D11CreateDevice = GetProcAddress("d3d11", "D3D11CreateDevice"); 
                    if (!D3D11CreateDevice) return false;

                    // Create D3D device and context
                    D3D_FEATURE_LEVEL featureLevels[] = {
                        D3D_FEATURE_LEVEL_11_0,
                        D3D_FEATURE_LEVEL_10_1,
                        D3D_FEATURE_LEVEL_10_0
                    };
                    
                    ID3D11Device* device = nullptr;
                    ID3D11DeviceContext* context = nullptr;
                    D3D_FEATURE_LEVEL selectedLevel;
                    
                    HRESULT hr = D3D11CreateDevice(
                        nullptr,                    // Default adapter
                        D3D_DRIVER_TYPE_HARDWARE,   // Hardware driver
                        nullptr,                    // Software driver (none)
                        0,                          // Flags
                        featureLevels,             // Feature levels
                        ARRAYSIZE(featureLevels),  // Num feature levels
                        D3D11_SDK_VERSION,         // SDK version
                        &device,                   // Output device
                        &selectedLevel,            // Selected feature level
                        &context                   // Output context
                    );
                    
                    if (FAILED(hr)) {
                        return false;
                    }
                    
                    // Create swap chain
                    IDXGIDevice* dxgiDevice = nullptr;
                    hr = device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
                    if (FAILED(hr)) {
                        device->Release();
                        context->Release();
                        return false;
                    }
                    
                    IDXGIAdapter* dxgiAdapter = nullptr;
                    hr = dxgiDevice->GetAdapter(&dxgiAdapter);
                    dxgiDevice->Release();
                    if (FAILED(hr)) {
                        device->Release();
                        context->Release();
                        return false;
                    }
                    
                    IDXGIFactory* dxgiFactory = nullptr;
                    hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory);
                    dxgiAdapter->Release();
                    if (FAILED(hr)) {
                        device->Release();
                        context->Release();
                        return false;
                    }
                    
                    // Configure and create swap chain
                    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
                    swapChainDesc.BufferCount = 2;
                    swapChainDesc.BufferDesc.Width = 0;  // Use window width
                    swapChainDesc.BufferDesc.Height = 0; // Use window height
                    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
                    swapChainDesc.OutputWindow = GetActiveWindow(); // Main window handle
                    swapChainDesc.SampleDesc.Count = 1;
                    swapChainDesc.SampleDesc.Quality = 0;
                    swapChainDesc.Windowed = TRUE;
                    
                    IDXGISwapChain* swapChain = nullptr;
                    hr = dxgiFactory->CreateSwapChain(device, &swapChainDesc, &swapChain);
                    dxgiFactory->Release();
                    
                    if (FAILED(hr)) {
                        device->Release();
                        context->Release();
                        return false;
                    }
                    
                    // Store device, context and swap chain as member variables
                    m_device = device;
                    m_context = context;
                    m_swapChain = swapChain;
                    
                    return true;
                }
                
                void validateModernAPIs() {
                    versionChecker.validateWin11Compatibility();
                    updateDirectXWrappers();
                    validateSystemCalls();
                }
                
                void updateDirectXWrappers() {
                    apiLayer.updateDX12Wrapper();
                    apiLayer.updateVulkanWrapper();
                    apiLayer.updateDXGIWrapper();
                }
                
                void Cleanup() {
                    if (dxgi) FreeLibrary(dxgi);
                    if (d3d12) FreeLibrary(d3d12);
                    if (d3d11) FreeLibrary(d3d11);
                    if (gdi32) FreeLibrary(gdi32);
                    if (user32) FreeLibrary(user32);
                    if (kernel32) FreeLibrary(kernel32);
                    if (m_device) m_device->Release();
                    if (m_context) m_context->Release();
                    if (m_swapChain) m_swapChain->Release();
                }

                ~WindowsCompatibilityLayer() {
                    Cleanup();
                }

            private:
                [[nodiscard]] std::expected initializeSubsystems(const InitializeParams& params);
            };

            class DirectXWrapper {
            private:
                std::shared_ptr device;
                std::unique_ptr commandQueue;
                std::vector featureLevels;
                
                IDXGIFactory* m_factory;
                IDXGIAdapter* m_adapter;
                ID3D12Debug* m_debugController;

            public:
                [[nodiscard]] std::expected initialize();
                [[nodiscard]] std::shared_ptr getDevice() const;
                
                void optimizeForGaming() {
                    device->SetGpuBasedValidation(false);
                    commandQueue->SetPriorityClass(D3D12_COMMAND_QUEUE_PRIORITY_REALTIME);
                    m_factory->SetHDRState(true);
                    m_adapter->SetGpuPreference(DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE);
                }
            };
            } // namespace Win32Compat
        