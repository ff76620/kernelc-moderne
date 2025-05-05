
#include 
#include 
#include 
#include 
#include 
#include 
#include 
#include 
#include 

#include "../memory/GraphicsResourceManager.hpp"
#include "../memory/shader_compiler.hpp"
#include "../memory/shader_types.hpp"

class EnhancedShaderManager {
private:
    struct CacheEntry {
        std::vector shaderBinary;
        std::string hash;
        time_t lastUsed;
        std::shared_ptr compiledShader;
    };
    
    std::unordered_map shaderCache;
    std::unique_ptr compiler;
    
    const size_t MAX_CACHE_SIZE = 1024;
    const time_t CACHE_TIMEOUT = 3600;

public:
    EnhancedShaderManager() {
        compiler = std::make_unique();
    }

    void initializeShaderSystem() {
        compiler = std::make_unique();
        loadDefaultShaders();
        cleanupCache();
    }

    std::shared_ptr getOrCompileShader(
        const std::string& source,
        ShaderType type,
        const std::string& shaderKey
    ) {
        if (auto cached = getCachedShader(shaderKey)) {
            return cached->compiledShader;
        }

        auto shader = compileShader(source, type);
        if (shader) {
            cacheShader(shaderKey, shader);
        }
        return shader;
    }

    std::shared_ptr compileShader(
        const std::string& source,
        ShaderType type
    ) {
        auto shader = compiler->compile(source, type);
        if (shader) {
            optimizeShader(shader);
        }
        return shader;
    }

    void optimizeShader(std::shared_ptr shader) {
        compiler->optimize(shader, OptimizationLevel::Maximum);
    }

private:
    std::string calculateHash(const std::vector& binary) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_CTX sha256;
        SHA256_Init(&sha256);
        SHA256_Update(&sha256, binary.data(), binary.size());
        SHA256_Final(hash, &sha256);

        std::stringstream ss;
        for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }
        return ss.str();
    }
    
    void loadDefaultShaders() {
        // Default vertex shader for basic 3D rendering
        const std::string defaultVertexShader = R"(
            #version 450
            layout(location = 0) in vec3 position;
            layout(location = 1) in vec2 texCoord;
            layout(location = 2) in vec3 normal;
            
            layout(location = 0) out vec2 fragTexCoord;
            layout(location = 1) out vec3 fragNormal;
            
            layout(binding = 0) uniform UniformBufferObject {
                mat4 model;
                mat4 view;
                mat4 proj;
            } ubo;
            
            void main() {
                gl_Position = ubo.proj * ubo.view * ubo.model * vec4(position, 1.0);
                fragTexCoord = texCoord;
                fragNormal = mat3(ubo.model) * normal;
            }
        )";

        // Default fragment shader with basic lighting
        const std::string defaultFragmentShader = R"(
            #version 450
            layout(location = 0) in vec2 fragTexCoord;
            layout(location = 1) in vec3 fragNormal;
            
            layout(location = 0) out vec4 outColor;
            
            layout(binding = 1) uniform sampler2D texSampler;
            
            void main() {
                vec3 lightDir = normalize(vec3(1.0, 1.0, 0.0));
                vec3 norm = normalize(fragNormal);
                float diff = max(dot(norm, lightDir), 0.0);
                
                vec3 ambient = vec3(0.1);
                vec3 diffuse = diff * vec3(1.0);
                
                vec4 texColor = texture(texSampler, fragTexCoord);
                outColor = vec4((ambient + diffuse) * texColor.rgb, texColor.a);
            }
        )";

        // Post-processing fragment shader
        const std::string postProcessShader = R"(
            #version 450
            layout(location = 0) in vec2 fragTexCoord;
            layout(location = 0) out vec4 outColor;
            
            layout(binding = 0) uniform sampler2D screenTexture;
            
            void main() {
                vec3 color = texture(screenTexture, fragTexCoord).rgb;
                
                // Apply basic tone mapping
                color = color / (color + vec3(1.0));
                
                // Apply gamma correction
                float gamma = 2.2;
                color = pow(color, vec3(1.0/gamma));
                
                outColor = vec4(color, 1.0);
            }
        )";

        // Shadow mapping vertex shader
        const std::string shadowVertexShader = R"(
            #version 450
            layout(location = 0) in vec3 position;
            
            layout(binding = 0) uniform UniformBufferObject {
                mat4 lightSpaceMatrix;
                mat4 model;
            } ubo;
            
            void main() {
                gl_Position = ubo.lightSpaceMatrix * ubo.model * vec4(position, 1.0);
            }
        )";

        // Compile and cache default shaders
        auto vertexShader = compileShader(defaultVertexShader, ShaderType::Vertex);
        auto fragmentShader = compileShader(defaultFragmentShader, ShaderType::Fragment);
        auto postProcessFragShader = compileShader(postProcessShader, ShaderType::Fragment);
        auto shadowVertShader = compileShader(shadowVertexShader, ShaderType::Vertex);

        if (vertexShader) cacheShader("default_vertex", vertexShader);
        if (fragmentShader) cacheShader("default_fragment", fragmentShader);
        if (postProcessFragShader) cacheShader("post_process", postProcessFragShader);
        if (shadowVertShader) cacheShader("shadow_vertex", shadowVertShader);
    }

    void cacheShader(const std::string& shaderKey, 
                    std::shared_ptr shader) {
        auto binary = shader->getBinary();
        CacheEntry entry{
            binary,
            calculateHash(binary),
            time(nullptr),
            shader
        };
        
        if (shaderCache.size() >= MAX_CACHE_SIZE) {
            removeOldestEntry();
        }
        
        shaderCache[shaderKey] = entry;
    }
    
    std::optional getCachedShader(
        const std::string& shaderKey) {
        if (auto it = shaderCache.find(shaderKey); it != shaderCache.end()) {
            it->second.lastUsed = time(nullptr);
            return &it->second;
        }
        return std::nullopt;
    }
    
    void cleanupCache() {
        time_t currentTime = time(nullptr);
        auto it = shaderCache.begin();
        while (it != shaderCache.end()) {
            if (currentTime - it->second.lastUsed > CACHE_TIMEOUT) {
                it = shaderCache.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    void removeOldestEntry() {
        auto oldest = shaderCache.begin();
        for (auto it = shaderCache.begin(); it != shaderCache.end(); ++it) {
            if (it->second.lastUsed < oldest->second.lastUsed) {
                oldest = it;
            }
        }
        if (oldest != shaderCache.end()) {
            shaderCache.erase(oldest);
        }
    }
};