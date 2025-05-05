
#pragma once

#include 
#include 

enum class ShaderType {
    Vertex,
    Fragment,
    Compute,
    Geometry,
    TessControl,
    TessEvaluation
};

enum class OptimizationLevel {
    None,
    Basic,
    Medium, 
    Maximum
};

struct ShaderCompileOptions {
    bool enableDebugInfo = false;
    bool enableOptimizations = true;
    OptimizationLevel optimizationLevel = OptimizationLevel::Medium;
    std::string entryPoint = "main";
    std::string targetProfile = ""; // e.g. "vs_5_0" for DirectX
};

class Shader {
public:
    virtual ~Shader() = default;
    
    virtual const std::string& getName() const = 0;
    virtual ShaderType getType() const = 0;
    virtual const std::vector& getBinary() const = 0;
    virtual bool isValid() const = 0;
    
    virtual bool setUniform(const std::string& name, const void* data, size_t size) = 0;
    virtual bool bind() = 0;
    virtual void unbind() = 0;
};