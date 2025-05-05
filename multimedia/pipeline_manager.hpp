#ifndef PIPELINE_MANAGER_HPP
#define PIPELINE_MANAGER_HPP

#include 
#include 
#include 
#include 
#include 
#include 
#include "../include/types.hpp"

namespace Kernel {
namespace Multimedia {

class PipelineManager {
public:
    enum class MediaType {
        Audio,
        Video,
        Image,
        Subtitle,
        Metadata
    };

    struct PipelineStage {
        std::string id;
        MediaType type;
        std::function processor;
        uint32_t bufferSize;
        bool enabled;
        double processingLoad;
    };

    struct Pipeline {
        std::string id;
        std::vector stages;
        bool active;
        uint32_t priority;
        double throughput;
        uint32_t latency;
    };

    PipelineManager();
    ~PipelineManager();

    // Pipeline setup and control
    bool initialize(uint32_t numThreads);
    void shutdown();
    bool createPipeline(const std::string& pipelineId, uint32_t priority);
    bool deletePipeline(const std::string& pipelineId);
    bool addStage(const std::string& pipelineId, const PipelineStage& stage);
    bool removeStage(const std::string& pipelineId, const std::string& stageId);

    // Data processing
    bool pushData(const std::string& pipelineId, 
                 const void* data, 
                 size_t size,
                 MediaType type);
    
    bool processPipeline(const std::string& pipelineId);
    void flushPipeline(const std::string& pipelineId);

    // Pipeline control
    void setPipelinePriority(const std::string& pipelineId, uint32_t priority);
    void enableStage(const std::string& pipelineId, const std::string& stageId);
    void disableStage(const std::string& pipelineId, const std::string& stageId);

    // Performance monitoring
    struct PipelineStats {
        double processingTime;
        double throughput;
        uint32_t activeStages;
        uint32_t queueSize;
        double stageLatencies[16];
    };

    PipelineStats getPipelineStats(const std::string& pipelineId) const;
    double getPipelineLatency(const std::string& pipelineId) const;

    // Resource management
    void setThreadCount(uint32_t count);
    void optimizeResourceUsage();
    void adjustBufferSizes(const std::string& pipelineId);

private:
    std::map> pipelines;
    std::vector workerThreads;
    std::queue> taskQueue;
    std::mutex mutex;
    std::condition_variable condition;
    bool running;

    // Internal processing
    void workerFunction();
    bool validatePipeline(const Pipeline& pipeline);
    void monitorPerformance();
    void balanceLoad();
    
    // Buffer management
    void resizeBuffers(Pipeline& pipeline);
    void cleanupBuffers();
    
    // Thread management
    void startWorkers(uint32_t count);
    void stopWorkers();
};

} // namespace Multimedia
} // namespace Kernel

#endif