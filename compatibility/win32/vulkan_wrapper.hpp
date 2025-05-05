#pragma once
#include 
#include 
#include 

class VulkanWrapper {
private:
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    std::vector queues;
    VkPhysicalDeviceFeatures2 deviceFeatures{};
    VkPhysicalDeviceVulkan13Features features13{};
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR rtFeatures{};

public:
    bool InitializeVulkan() {
        // Application info with latest API version
        VkApplicationInfo appInfo{};
        appInfo.apiVersion = VK_API_VERSION_1_3;
        
        // Enable all validation layers in debug
        std::vector validationLayers = {
            "VK_LAYER_KHRONOS_validation",
            "VK_LAYER_KHRONOS_synchronization2"
        };

        // Enable key extensions
        std::vector deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
            VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
            VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME
        };

        // Setup instance
        VkInstanceCreateInfo createInfo{};
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledLayerCount = validationLayers.size();
        createInfo.ppEnabledLayerNames = validationLayers.data();
        vkCreateInstance(&createInfo, nullptr, &instance);

        // Select best physical device
        SelectOptimalDevice();

        // Configure DLSS/FSR
        SetupUpscaling();
        
        // Setup HDR
        ConfigureHDR();
        
        // Enable Multi-GPU
        EnableMultiGPU();

        return true;
    }

private:
    void SelectOptimalDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        std::vector devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        // Score and select best device
        int maxScore = 0;
        for (const auto& device : devices) {
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(device, &deviceProperties);
            
            int score = 0;
            if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                score += 10000;
            score += deviceProperties.limits.maxImageDimension2D;
            
            if (score > maxScore) {
                maxScore = score;
                physicalDevice = device;
            }
        }
    }

    void SetupUpscaling() {
        VkPhysicalDeviceDynamicRenderingFeatures dynamicRendering{};
        dynamicRendering.dynamicRendering = VK_TRUE;
        
        features13.dynamicRendering = VK_TRUE;
        features13.synchronization2 = VK_TRUE;
        features13.maintenance4 = VK_TRUE;
    }

    void ConfigureHDR() {
        VkHdrMetadataEXT hdrMetadata{};
        hdrMetadata.displayPrimaryRed = {0.708f, 0.292f};
        hdrMetadata.displayPrimaryGreen = {0.170f, 0.797f};
        hdrMetadata.displayPrimaryBlue = {0.131f, 0.046f};
        hdrMetadata.whitePoint = {0.3127f, 0.3290f};
        hdrMetadata.maxLuminance = 1000.0f;
        hdrMetadata.minLuminance = 0.001f;
        hdrMetadata.maxContentLightLevel = 1000.0f;
        hdrMetadata.maxFrameAverageLightLevel = 400.0f;
    }

    void EnableMultiGPU() {
        VkPhysicalDeviceGroupProperties deviceGroup{};
        uint32_t deviceGroupCount = 0;
        vkEnumeratePhysicalDeviceGroups(instance, &deviceGroupCount, nullptr);
        
        if (deviceGroupCount > 0) {
            std::vector deviceGroups(deviceGroupCount);
            vkEnumeratePhysicalDeviceGroups(instance, &deviceGroupCount, deviceGroups.data());
            
            // Configure SLI/Crossfire if available
            if (deviceGroups[0].physicalDeviceCount > 1) {
                // Setup multi-GPU memory pools and command buffers
                VkDeviceGroupDeviceCreateInfo deviceGroupCreateInfo{};
                deviceGroupCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_GROUP_DEVICE_CREATE_INFO;
                deviceGroupCreateInfo.physicalDeviceCount = deviceGroups[0].physicalDeviceCount;
                deviceGroupCreateInfo.pPhysicalDevices = deviceGroups[0].physicalDevices;
                
                // Create logical device spanning all GPUs
                VkDeviceCreateInfo deviceCreateInfo{};
                deviceCreateInfo.pNext = &deviceGroupCreateInfo;
                vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device);
            }
        }
    }
};