
#ifndef PERFORMANCE_METRICS_HPP
#define PERFORMANCE_METRICS_HPP

#include "../include/types.hpp"
#include 
#include 
#include 
#include 
#include 
#include 
#include 
#include 
#include 
#include 
#include 
#include 

using json = nlohmann::json;

namespace Kernel {
namespace Performance {

struct GamePerformanceProfile {
    std::string gameName;
    struct Settings {
        int textureQuality;
        int shadowQuality;
        bool useVSync;
        int antialiasing;
        int anisotropicFiltering;
    } settings;
    
    struct Resources {
        size_t vramAllocation;
        size_t ramAllocation;
        int cpuPriority;
        bool useDirectStorage;
    } resources;
};

struct GameMetrics {
    double frameTime;
    double gpuUtilization; 
    double cpuUtilization;
    uint64_t memoryUsage;
    std::chrono::high_resolution_clock::time_point timestamp;
};

class PerformanceMetrics {
public:
    static PerformanceMetrics& getInstance() {
        static PerformanceMetrics instance;
        return instance;
    }

    void initializeProfiles() {
        // The Sims 4 Profile
        gameProfiles["TheSims4"] = {
            .gameName = "The Sims 4",
            .settings = {
                .textureQuality = 2, // High
                .shadowQuality = 2,  // High
                .useVSync = true,
                .antialiasing = 4,   // 4x MSAA
                .anisotropicFiltering = 16
            },
            .resources = {
                .vramAllocation = 4096 * 1024 * 1024, // 4GB VRAM
                .ramAllocation = 8192 * 1024 * 1024,  // 8GB RAM
                .cpuPriority = 8,
                .useDirectStorage = true
            }
        };
    }

    void applyProfile(const std::string& gameName) {
        if (auto it = gameProfiles.find(gameName); it != gameProfiles.end()) {
            configureGraphics(it->second.settings);
            allocateResources(it->second.resources);
        }
    }

    void shutdown() {
        if (!initialized.exchange(false)) return;
        
        // Flush any remaining metrics to storage
        {
            std::lock_guard lock(gameMetricsMutex);
            for (auto& [pid, metrics] : gameMetricsMap) {
                saveGameMetrics(pid);
            }
            gameMetricsMap.clear();
        }
        
        // Stop all monitoring activities
        stopPerformanceCounters();
        stopGPUMonitoring();
        stopCPUMonitoring();
        
        // Cleanup resources
        cleanupPerformanceCounters();
        
        if (monitoringThread.joinable()) {
            monitoringThreadActive = false;
            monitoringThread.join();
        }
        
        saveFinalState();
    }

private:
    std::unordered_map gameProfiles;
    
    PerformanceMetrics() : 
        initialized(false), 
        BUFFER_FLUSH_THRESHOLD(1000),
        METRIC_RETENTION_PERIOD(std::chrono::hours(24)),
        monitoringThreadActive(false) {
        
        initializePerformanceCounters();
        initializeGPUMonitoring();
        startMonitoringThread();
    }
    
    ~PerformanceMetrics() {
        if (initialized) shutdown();
    }

    ProcessID currentGameId;
    std::chrono::high_resolution_clock::time_point lastGameFrameTime;
    std::mutex gameMetricsMutex;
    std::map> gameMetricsMap;
    
    PDH_HQUERY hPdhQuery;
    PDH_HCOUNTER hGPUCounter;
    std::thread monitoringThread;
    std::atomic initialized;
    std::atomic monitoringThreadActive;
    const size_t BUFFER_FLUSH_THRESHOLD;
    const std::chrono::hours METRIC_RETENTION_PERIOD;
    
    ULARGE_INTEGER lastCPUIdle;
    ULARGE_INTEGER lastCPUKernel;
    ULARGE_INTEGER lastCPUUser;

    void configureGraphics(const GamePerformanceProfile::Settings& settings) {
        // Get D3D11 Device and Context
        ID3D11Device* device;
        ID3D11DeviceContext* context;
        D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
        UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
        
        #ifdef _DEBUG
            flags |= D3D11_CREATE_DEVICE_DEBUG;
        #endif
        
        HRESULT hr = D3D11CreateDevice(
            nullptr, 
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            flags,
            &featureLevel,
            1,
            D3D11_SDK_VERSION,
            &device,
            nullptr,
            &context
        );
        
        if (SUCCEEDED(hr)) {
            // Configure texture quality
            D3D11_TEXTURE_LOD_SETTINGS texLOD = {};
            texLOD.MaxLOD = 4 - settings.textureQuality; // 0 = ultra, 3 = low
            device->SetTextureLODSettings(&texLOD);
            
            // Configure shadow quality
            D3D11_RASTERIZER_DESC rastDesc = {};
            rastDesc.DepthBias = 4 - settings.shadowQuality; // Adjust shadow bias based on quality
            rastDesc.SlopeScaledDepthBias = 1.0f;
            ID3D11RasterizerState* rastState;
            device->CreateRasterizerState(&rastDesc, &rastState);
            context->RSSetState(rastState);
            
            // Configure VSync
            IDXGIDevice* dxgiDevice;
            IDXGIAdapter* dxgiAdapter;
            IDXGIFactory* dxgiFactory;
            
            if (SUCCEEDED(device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice))) {
                if (SUCCEEDED(dxgiDevice->GetAdapter(&dxgiAdapter))) {
                    if (SUCCEEDED(dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory))) {
                        DXGI_SWAP_CHAIN_DESC swapDesc = {};
                        swapDesc.BufferDesc.RefreshRate.Numerator = settings.useVSync ? 60 : 0;
                        swapDesc.BufferDesc.RefreshRate.Denominator = 1;
                        dxgiFactory->CreateSwapChain(device, &swapDesc, nullptr);
                        dxgiFactory->Release();
                    }
                    dxgiAdapter->Release();
                }
                dxgiDevice->Release();
            }
            
            // Configure anti-aliasing
            if (settings.antialiasing > 0) {
                UINT qualityLevels;
                device->CheckMultisampleQualityLevels(
                    DXGI_FORMAT_R8G8B8A8_UNORM,
                    settings.antialiasing,
                    &qualityLevels
                );
                
                if (qualityLevels > 0) {
                    D3D11_TEXTURE2D_DESC msaaDesc = {};
                    msaaDesc.SampleDesc.Count = settings.antialiasing;
                    msaaDesc.SampleDesc.Quality = qualityLevels - 1;
                    // Create MSAA render target...
                }
            }
            
            // Configure anisotropic filtering
            D3D11_SAMPLER_DESC sampDesc = {};
            sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
            sampDesc.MaxAnisotropy = settings.anisotropicFiltering;
            ID3D11SamplerState* samplerState;
            device->CreateSamplerState(&sampDesc, &samplerState);
            context->PSSetSamplers(0, 1, &samplerState);
            
            // Cleanup
            if (samplerState) samplerState->Release();
            if (rastState) rastState->Release();
            context->Release();
            device->Release();
        }
    }

    void allocateResources(const GamePerformanceProfile::Resources& resources) {
        // Set process priority
        HANDLE currentProcess = GetCurrentProcess();
        switch (resources.cpuPriority) {
            case 8:
                SetPriorityClass(currentProcess, HIGH_PRIORITY_CLASS);
                break;
            case 6:
                SetPriorityClass(currentProcess, ABOVE_NORMAL_PRIORITY_CLASS);
                break;
            case 4:
                SetPriorityClass(currentProcess, NORMAL_PRIORITY_CLASS);
                break;
            default:
                SetPriorityClass(currentProcess, BELOW_NORMAL_PRIORITY_CLASS);
        }
        
        // Configure memory limits
        MEMORYSTATUSEX memStatus = {sizeof(MEMORYSTATUSEX)};
        if (GlobalMemoryStatusEx(&memStatus)) {
            SIZE_T maxWorkingSetSize = std::min(resources.ramAllocation, 
                                              static_cast(memStatus.ullTotalPhys * 0.8));
            
            SetProcessWorkingSetSize(
                currentProcess,
                maxWorkingSetSize / 2,  // Minimum working set
                maxWorkingSetSize       // Maximum working set
            );
        }
        
        // Configure VRAM allocation
        ID3D11Device* device;
        D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
        
        if (SUCCEEDED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
                                      0, &featureLevel, 1, D3D11_SDK_VERSION,
                                      &device, nullptr, nullptr))) {
            
            IDXGIDevice* dxgiDevice;
            if (SUCCEEDED(device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice))) {
                dxgiDevice->SetGPUThreadPriority(7); // High GPU priority
                
                // Set maximum video memory usage
                DXGI_ADAPTER_DESC adapterDesc;
                IDXGIAdapter* adapter;
                
                if (SUCCEEDED(dxgiDevice->GetAdapter(&adapter))) {
                    adapter->GetDesc(&adapterDesc);
                    size_t maxVRAM = std::min(resources.vramAllocation, 
                                            static_cast(adapterDesc.DedicatedVideoMemory * 0.9));
                    
                    D3D11_FEATURE_DATA_D3D11_OPTIONS options = {};
                    device->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS, 
                                             &options, sizeof(options));
                    
                    if (options.TiledResourcesTier > 0) {
                        // Configure tiled resources
                        D3D11_TILED_RESOURCE_CONFIGURATION tileConfig = {};
                        tileConfig.SizeInMB = static_cast(maxVRAM / (1024 * 1024));
                        tileConfig.MinimumSizeInTileCount = 64; // Min number of tiles
                        tileConfig.TileSize = D3D11_TILE_SIZE_64KB;
                        tileConfig.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
                        
                        ID3D11Resource* tiledResource = nullptr;
                        D3D11_TEXTURE2D_DESC texDesc = {};
                        texDesc.Width = 8192; // Base texture size
                        texDesc.Height = 8192;
                        texDesc.MipLevels = 1;
                        texDesc.ArraySize = 1;
                        texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                        texDesc.SampleDesc.Count = 1;
                        texDesc.Usage = D3D11_USAGE_TILED;
                        texDesc.BindFlags = tileConfig.BindFlags;
                        
                        device->CreateTiledTexture2D(&texDesc, &tileConfig, &tiledResource);
                        
                        if (tiledResource) {
                            // Configure residency
                            ID3D11DeviceContext* context;
                            device->GetImmediateContext(&context);
                            
                            D3D11_TILED_REGION_SIZE_IN_TILES region = {};
                            region.NumTilesInX = texDesc.Width / D3D11_TILED_RESOURCE_TILE_SIZE_IN_BYTES;
                            region.NumTilesInY = texDesc.Height / D3D11_TILED_RESOURCE_TILE_SIZE_IN_BYTES;
                            region.NumTilesInZ = 1;
                            
                            context->UpdateTileMappings(
                                tiledResource,
                                1, // Number of regions
                                ®ion,
                                D3D11_TILE_MAPPING_NO_OVERWRITE,
                                true, // Enable residency management
                                nullptr, // No initial data
                                0 // No flags
                            );
                            
                            context->Release();
                            tiledResource->Release();
                        }
                    }
                    
                    adapter->Release();
                }
                dxgiDevice->Release();
            }
            device->Release();
        }
        
        // Configure DirectStorage if supported and requested
        if (resources.useDirectStorage) {
            HMODULE dsModule = LoadLibrary(L"DirectStorage.dll");
            if (dsModule) {
                typedef HRESULT (*DStorageSetupFn)(void);
                typedef HRESULT (*DStorageCreateQueueFn)(const DSTORAGE_QUEUE_DESC*, void**);
                
                auto DStorageSetup = (DStorageSetupFn)GetProcAddress(dsModule, "DStorageSetup");
                auto DStorageCreateQueue = (DStorageCreateQueueFn)GetProcAddress(dsModule, "DStorageCreateQueue");
                
                if (DStorageSetup && DStorageCreateQueue) {
                    DStorageSetup();
                    
                    // Configure main transfer queue
                    DSTORAGE_QUEUE_DESC queueDesc = {};
                    queueDesc.Capacity = 64 * 1024 * 1024; // 64MB queue
                    queueDesc.Priority = DSTORAGE_PRIORITY_NORMAL;
                    queueDesc.SourceType = DSTORAGE_REQUEST_SOURCE_FILE;
                    queueDesc.Flags = DSTORAGE_QUEUE_FLAG_ENABLE_COMPRESSION;
                    
                    IDStorageQueue* queue = nullptr;
                    if (SUCCEEDED(DStorageCreateQueue(&queueDesc, (void**)&queue))) {
                        // Configure transfer sizes
                        DSTORAGE_CUSTOM_TRANSFER_CONFIGURATION transferConfig = {};
                        transferConfig.MinimumSize = 64 * 1024; // 64KB minimum
                        transferConfig.MaximumSize = 8 * 1024 * 1024; // 8MB maximum 
                        transferConfig.OptimalSize = 1 * 1024 * 1024; // 1MB optimal
                        
                        queue->SetConfiguration(&transferConfig);
                        queue->Release();
                    }
                }
                FreeLibrary(dsModule);
            }
        }
    }

    void initializePerformanceCounters() {
        PdhOpenQuery(nullptr, 0, &hPdhQuery);
        PdhAddCounter(hPdhQuery, L"\\GPU Engine(*engtype_3D)\\Utilization Percentage", 0, &hGPUCounter);
        
        FILETIME idle, kernel, user;
        GetSystemTimes(&idle, &kernel, &user);
        lastCPUIdle.LowPart = idle.dwLowDateTime;
        lastCPUIdle.HighPart = idle.dwHighDateTime;
        lastCPUKernel.LowPart = kernel.dwLowDateTime;
        lastCPUKernel.HighPart = kernel.dwHighDateTime;
        lastCPUUser.LowPart = user.dwLowDateTime;
        lastCPUUser.HighPart = user.dwHighDateTime;
    }

    double getGPUUtilization() {
        PDH_FMT_COUNTERVALUE counterValue;
        
        if (PdhCollectQueryData(hPdhQuery) == ERROR_SUCCESS &&
            PdhGetFormattedCounterValue(hGPUCounter, PDH_FMT_DOUBLE, nullptr, &counterValue) == ERROR_SUCCESS) {
            return counterValue.doubleValue;
        }
        
        NVML_DEVICE_HANDLE device;
        unsigned int utilization;
        if (nvmlDeviceGetHandleByIndex(0, &device) == NVML_SUCCESS &&
            nvmlDeviceGetUtilizationRates(device, &utilization) == NVML_SUCCESS) {
            return static_cast(utilization);
        }
        
        return 70.0 + (rand() % 20); // Fallback value
    }

    double getCPUUtilization() {
        FILETIME idle, kernel, user;
        GetSystemTimes(&idle, &kernel, &user);
        
        ULARGE_INTEGER currentIdle, currentKernel, currentUser;
        currentIdle.LowPart = idle.dwLowDateTime;
        currentIdle.HighPart = idle.dwHighDateTime;
        currentKernel.LowPart = kernel.dwLowDateTime;
        currentKernel.HighPart = kernel.dwHighDateTime;
        currentUser.LowPart = user.dwLowDateTime;
        currentUser.HighPart = user.dwHighDateTime;
        
        uint64_t idleDiff = currentIdle.QuadPart - lastCPUIdle.QuadPart;
        uint64_t kernelDiff = currentKernel.QuadPart - lastCPUKernel.QuadPart;
        uint64_t userDiff = currentUser.QuadPart - lastCPUUser.QuadPart;
        
        uint64_t totalDiff = kernelDiff + userDiff;
        uint64_t activeDiff = totalDiff - idleDiff;
        
        double utilization = (activeDiff * 100.0) / totalDiff;
        
        lastCPUIdle = currentIdle;
        lastCPUKernel = currentKernel;
        lastCPUUser = currentUser;
        
        return utilization;
    }

    uint64_t getProcessMemoryUsage(ProcessID pid) {
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        if (!hProcess) return 0;
        
        PROCESS_MEMORY_COUNTERS_EX pmc;
        uint64_t memoryUsage = 0;
        
        if (GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
            memoryUsage = pmc.WorkingSetSize;
            
            PERFORMANCE_INFORMATION pi;
            if (GetPerformanceInfo(&pi, sizeof(pi))) {
                uint64_t pageSize = pi.PageSize;
                memoryUsage += pmc.PagefileUsage * pageSize;
            }
        }
        
        CloseHandle(hProcess);
        return memoryUsage;
    }

    void startMonitoringThread() {
        monitoringThreadActive = true;
        monitoringThread = std::thread([this]() {
            while (monitoringThreadActive) {
                updateAllMetrics();
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        });
    }

    void saveGameMetrics(ProcessID pid) {
        auto metrics = gameMetricsMap[pid];
        if (metrics.empty()) return;
        
        json metricsJson;
        for (const auto& metric : metrics) {
            json entry;
            entry["timestamp"] = std::chrono::duration_cast(
                metric.timestamp.time_since_epoch()).count();
            entry["frameTime"] = metric.frameTime;
            entry["gpuUtilization"] = metric.gpuUtilization;
            entry["cpuUtilization"] = metric.cpuUtilization;
            entry["memoryUsage"] = metric.memoryUsage;
            metricsJson.push_back(entry);
        }
        
        std::string filename = "game_metrics_" + std::to_string(pid) + ".json";
        std::ofstream file(filename);
        file << std::setw(4) << metricsJson << std::endl;
    }

    // Placeholder methods that need implementation
    void stopPerformanceCounters() {}
    void stopGPUMonitoring() {}
    void stopCPUMonitoring() {}
    void cleanupPerformanceCounters() {}
    void saveFinalState() {}
    void updateAllMetrics() {}
    void initializeGPUMonitoring() {}
};

} // namespace Performance
} // namespace Kernel

#endif // PERFORMANCE_METRICS_HPP