
#pragma once

#include 
#include 
#include 
#include 
#include 
#include 
#include "../memory/shader_types.hpp"

class ShaderCompiler {
public:
    ShaderCompiler() = default;
    virtual ~ShaderCompiler() = default;

    struct CacheEntry {
        std::vector shaderBinary;
        std::string hash;
        time_t lastUsed;
    };

    std::unordered_map shaderCache;

    std::shared_ptr compile(
        const std::string& source,
        ShaderType type,
        const ShaderCompileOptions& options = {}
    ) {
        // Check cache first
        std::string cacheKey = source + std::to_string(static_cast(type));
        auto cachedShader = getCachedShader(cacheKey);
        if (cachedShader) {
            return createShaderFromBinary(*cachedShader, type);
        }

        // Précompilation et validation du code source
        if (!validateSource(source)) {
            return nullptr;
        }

        // Compilation selon le type de shader
        std::shared_ptr shader = nullptr;
        switch (type) {
            case ShaderType::Vertex:
                shader = compileVertexShader(source, options);
                break;
            case ShaderType::Fragment:
                shader = compileFragmentShader(source, options);
                break;
            case ShaderType::Compute:
                shader = compileComputeShader(source, options);
                break;
            case ShaderType::Geometry:
                shader = compileGeometryShader(source, options);
                break;
            case ShaderType::TessControl:
                shader = compileTessControlShader(source, options);
                break;
            case ShaderType::TessEvaluation:
                shader = compileTessEvalShader(source, options);
                break;
        }

        if (shader && shader->isValid()) {
            cacheShader(cacheKey, shader->getBinary());
        }

        return shader;
    }

    void optimize(std::shared_ptr shader, OptimizationLevel level) {
        if (!shader || !shader->isValid()) {
            return;
        }

        switch (level) {
            case OptimizationLevel::Maximum:
                applyMaxOptimizations(shader);
                break;
            case OptimizationLevel::Medium:
                applyMediumOptimizations(shader);
                break;
            case OptimizationLevel::Basic:
                applyBasicOptimizations(shader);
                break;
            case OptimizationLevel::None:
                break;
        }
    }

private:
    std::string calculateHash(const std::vector& binary) {
        uint32_t hash = 0x811C9DC5; // FNV offset basis
        for (auto byte : binary) {
            hash ^= byte;
            hash *= 0x01000193; // FNV prime
        }
        return std::to_string(hash);
    }

    void cacheShader(const std::string& shaderKey, const std::vector& binary) {
        CacheEntry entry{binary, calculateHash(binary), time(nullptr)};
        shaderCache[shaderKey] = entry;
    }

    std::optional> getCachedShader(const std::string& shaderKey) {
        if (auto it = shaderCache.find(shaderKey); it != shaderCache.end()) {
            it->second.lastUsed = time(nullptr);
            return it->second.shaderBinary;
        }
        return std::nullopt;
    }

    bool validateSource(const std::string& source) {
        if (source.empty()) {
            return false;
        }
        return checkShaderSyntax(source);
    }

    std::shared_ptr compileVertexShader(
        const std::string& source,
        const ShaderCompileOptions& options
    ) {
        std::vector binary = compileToSpirV(source, ShaderType::Vertex, options);
        if (!binary.empty()) {
            return createShaderFromBinary(binary, ShaderType::Vertex);
        }
        return nullptr;
    }

    std::shared_ptr compileFragmentShader(
        const std::string& source,
        const ShaderCompileOptions& options
    ) {
        std::vector binary = compileToSpirV(source, ShaderType::Fragment, options);
        if (!binary.empty()) {
            return createShaderFromBinary(binary, ShaderType::Fragment);
        }
        return nullptr;
    }

    std::shared_ptr compileComputeShader(
        const std::string& source,
        const ShaderCompileOptions& options
    ) {
        std::vector binary = compileToSpirV(source, ShaderType::Compute, options);
        if (!binary.empty()) {
            return createShaderFromBinary(binary, ShaderType::Compute);
        }
        return nullptr;
    }

    std::shared_ptr compileGeometryShader(
        const std::string& source,
        const ShaderCompileOptions& options
    ) {
        std::vector binary = compileToSpirV(source, ShaderType::Geometry, options);
        if (!binary.empty()) {
            return createShaderFromBinary(binary, ShaderType::Geometry);
        }
        return nullptr;
    }

    std::shared_ptr compileTessControlShader(
        const std::string& source,
        const ShaderCompileOptions& options
    ) {
        std::vector binary = compileToSpirV(source, ShaderType::TessControl, options);
        if (!binary.empty()) {
            return createShaderFromBinary(binary, ShaderType::TessControl);
        }
        return nullptr;
    }

    std::shared_ptr compileTessEvalShader(
        const std::string& source,
        const ShaderCompileOptions& options
    ) {
        std::vector binary = compileToSpirV(source, ShaderType::TessEvaluation, options);
        if (!binary.empty()) {
            return createShaderFromBinary(binary, ShaderType::TessEvaluation);
        }
        return nullptr;
    }

    bool checkShaderSyntax(const std::string& source) {
        bool hasMain = source.find("void main") != std::string::npos;
        bool hasValidBraces = 0;
        for (char c : source) {
            if (c == '{') hasValidBraces++;
            if (c == '}') hasValidBraces--;
        }
        return hasMain && (hasValidBraces == 0);
    }

    std::vector compileToSpirV(
        const std::string& source,
        ShaderType type,
        const ShaderCompileOptions& options
    ) {
        glslang::InitializeProcess();

        EShLanguage stage;
        switch(type) {
            case ShaderType::Vertex: 
                stage = EShLangVertex; break;
            case ShaderType::Fragment:
                stage = EShLangFragment; break;
            case ShaderType::Compute:
                stage = EShLangCompute; break;
            case ShaderType::Geometry:
                stage = EShLangGeometry; break;
            case ShaderType::TessControl:
                stage = EShLangTessControl; break;
            case ShaderType::TessEvaluation:
                stage = EShLangTessEvaluation; break;
            default:
                return std::vector();
        }

        glslang::TShader shader(stage);

        const char* shaderStrings[1] = {source.c_str()};
        shader.setStrings(shaderStrings, 1);

        TBuiltInResource resources = DefaultTBuiltInResource;
        
        EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);
        if (options.enableDebugInfo) {
            messages = (EShMessages)(messages | EShMsgDebugInfo);
        }

        if (!shader.parse(&resources, 100, false, messages)) {
            return std::vector();
        }

        glslang::TProgram program;
        program.addShader(&shader);
        if (!program.link(messages)) {
            return std::vector();
        }

        std::vector spirv;
        spv::SpvBuildLogger logger;
        glslang::GlslangToSpv(*program.getIntermediate(stage), spirv, &logger);

        std::vector binary;
        binary.resize(spirv.size() * sizeof(uint32_t));
        memcpy(binary.data(), spirv.data(), binary.size());

        glslang::FinalizeProcess();

        return binary;
    }

    std::shared_ptr createShaderFromBinary(
        const std::vector& binary,
        ShaderType type
    ) {
        class VulkanShader : public Shader {
            VkShaderModule shaderModule;
            std::string name;
            ShaderType shaderType;
            std::vector binaryData;
            VkDevice device;
            bool valid;

        public:
            VulkanShader(VkDevice dev, const std::vector& data, ShaderType t) 
                : device(dev), shaderType(t), binaryData(data) {
                
                name = "Shader_" + std::to_string(reinterpret_cast(this));
                valid = createShaderModule();
            }

            ~VulkanShader() {
                if (valid) {
                    vkDestroyShaderModule(device, shaderModule, nullptr);
                }
            }

            bool createShaderModule() {
                VkShaderModuleCreateInfo createInfo{};
                createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
                createInfo.codeSize = binaryData.size();
                createInfo.pCode = reinterpret_cast(binaryData.data());

                if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
                    return false;
                }
                return true;
            }

            const std::string& getName() const override { return name; }
            ShaderType getType() const override { return shaderType; }
            const std::vector& getBinary() const override { return binaryData; }
            bool isValid() const override { return valid; }

            bool setUniform(const std::string& name, const void* data, size_t size) override {
                VkWriteDescriptorSet descriptorWrite{};
                return true;
            }

            bool bind() override {
                VkPipelineShaderStageCreateInfo shaderStageInfo{};
                shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                shaderStageInfo.stage = getVkShaderStage(shaderType);
                shaderStageInfo.module = shaderModule;
                shaderStageInfo.pName = "main";
                return true;
            }

            void unbind() override {
            }

        private:
            VkShaderStageFlagBits getVkShaderStage(ShaderType type) {
                switch(type) {
                    case ShaderType::Vertex: return VK_SHADER_STAGE_VERTEX_BIT;
                    case ShaderType::Fragment: return VK_SHADER_STAGE_FRAGMENT_BIT;
                    case ShaderType::Compute: return VK_SHADER_STAGE_COMPUTE_BIT;
                    case ShaderType::Geometry: return VK_SHADER_STAGE_GEOMETRY_BIT;
                    case ShaderType::TessControl: return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
                    case ShaderType::TessEvaluation: return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
                    default: return VK_SHADER_STAGE_VERTEX_BIT;
                }
            }
        };

        VkDevice device = getVulkanDevice();
        return std::make_shared(device, binary, type);
    }

    void applyBasicOptimizations(std::shared_ptr shader) {
        if (!shader || !shader->isValid()) return;
        
        auto binary = shader->getBinary();
    }

    void applyMediumOptimizations(std::shared_ptr shader) {
        applyBasicOptimizations(shader);
    }

    void applyMaxOptimizations(std::shared_ptr shader) {
        applyMediumOptimizations(shader);
    }
};

VkDevice getVulkanDevice() {
    static VkDevice device = nullptr;
    if (!device) {
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Shader Compiler";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        VkInstance instance;
        vkCreateInstance(&createInfo, nullptr, &instance);

        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        std::vector devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
        VkPhysicalDevice physicalDevice = devices[0];

        VkDeviceCreateInfo deviceInfo{};
        deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &device);
    }
    return device;
}