
#include 
#include 
#include 
#include 
#include 
#include 
#include 

class EnhancedGraphicsResourceManager {
private:
    struct MemoryBlock {
        void* address;
        size_t size;
        bool isFree;
    };

    struct MemoryPool {
        size_t totalSize;
        size_t usedSize; 
        std::vector blocks;
    };

    struct ResourceTracker {
        void* resource;
        uint64_t lastUseTime;
        size_t size;
        bool isInUse;
        bool isPriority;
    };

    enum class ResourceType {
        TEXTURE,
        VERTEX_BUFFER,
        INDEX_BUFFER,
        SHADER,
        RENDER_TARGET,
        OTHER
    };

    struct ResourceInfo {
        ResourceType type;
        union {
            struct {
                ID3D11Texture2D* texture;
                ID3D11ShaderResourceView* srv;
            } textureData;
            struct {
                ID3D11Buffer* buffer;
            } bufferData;
            struct {
                ID3D11DeviceChild* shader;
            } shaderData;
            struct {
                ID3D11RenderTargetView* rtv;
                ID3D11Texture2D* texture;
            } renderTargetData;
            void* genericResource;
        };
    };

    // Memory pools
    MemoryPool dedicatedVRAM;
    MemoryPool sharedVRAM;

    // Cache and resource management
    static const size_t CACHE_SIZE = 2048 * 1024 * 1024; // 2GB cache
    std::unordered_map> textureCache;
    std::unordered_map> shaderCache;
    std::unordered_map resourceInfoMap;
    std::unordered_map gpuCache;
    std::vector resources;
    std::mutex resourceMutex;

    // Constants and limits
    const uint64_t RESOURCE_TIMEOUT = 5000; // 5 seconds
    size_t currentCacheSize = 0;

public:
    void optimizeMemoryAllocation() {
        std::lock_guard lock(resourceMutex);
        defragmentMemory();
        compactResources();
        prioritizeGameTextures();
    }
    
    void allocateGameResources(size_t requiredMemory) {
        std::lock_guard lock(resourceMutex);
        if (dedicatedVRAM.usedSize + requiredMemory > dedicatedVRAM.totalSize) {
            moveToSharedMemory(getLowPriorityResources());
        }
        reserveMemoryBlock(requiredMemory);
    }

    void setCachePriority(uint64_t resourceId, bool isPriority) {
        std::lock_guard lock(resourceMutex);
        if(gpuCache.find(resourceId) != gpuCache.end()) {
            gpuCache[resourceId].isPriority = isPriority;
        }
    }
    
    void preloadResource(uint64_t resourceId, void* data, size_t size) {
        std::lock_guard lock(resourceMutex);
        if(currentCacheSize + size > CACHE_SIZE) {
            evictLeastUsed();
        }
        ResourceTracker tracker = {
            data,
            getCurrentTimestamp(),
            size,
            true,
            false
        };
        gpuCache[resourceId] = tracker;
        currentCacheSize += size;
    }

    std::shared_ptr loadTexture(const std::string& path) {
        std::lock_guard lock(resourceMutex);
        
        if (textureCache.find(path) != textureCache.end()) {
            markResourceUsed(textureCache[path].get());
            return textureCache[path];
        }

        auto texture = createTextureResource(path);
        textureCache[path] = texture;
        registerResource(texture.get(), texture->getSize());
        return texture;
    }

    void preloadResources(const std::vector& resourceList) {
        for (const auto& resource : resourceList) {
            loadTexture(resource);
        }
    }

    void collectGarbage() {
        std::lock_guard lock(resourceMutex);
        uint64_t currentTime = getCurrentTimestamp();
        
        auto it = resources.begin();
        while (it != resources.end()) {
            if (!it->isInUse && !it->isPriority && 
                (currentTime - it->lastUseTime) > RESOURCE_TIMEOUT) {
                releaseResource(it->resource);
                currentCacheSize -= it->size;
                it = resources.erase(it);
            } else {
                ++it;
            }
        }
    }

private:
    void evictLeastUsed() {
        uint64_t oldestTime = UINT64_MAX;
        uint64_t targetId = 0;
        
        for(const auto& [id, resource] : gpuCache) {
            if(!resource.isPriority && resource.lastUseTime < oldestTime) {
                oldestTime = resource.lastUseTime;
                targetId = id;
            }
        }
        
        if(targetId != 0) {
            currentCacheSize -= gpuCache[targetId].size;
            gpuCache.erase(targetId);
        }
    }

    void releaseResource(void* resource) {
        auto it = resourceInfoMap.find(resource);
        if (it == resourceInfoMap.end()) return;

        ResourceInfo& info = it->second;
        switch(info.type) {
            case ResourceType::TEXTURE:
                if (info.textureData.srv) info.textureData.srv->Release();
                if (info.textureData.texture) info.textureData.texture->Release();
                break;
            case ResourceType::VERTEX_BUFFER:
            case ResourceType::INDEX_BUFFER:
                if (info.bufferData.buffer) info.bufferData.buffer->Release();
                break;
            case ResourceType::SHADER:
                if (info.shaderData.shader) info.shaderData.shader->Release();
                break;
            case ResourceType::RENDER_TARGET:
                if (info.renderTargetData.rtv) info.renderTargetData.rtv->Release();
                if (info.renderTargetData.texture) info.renderTargetData.texture->Release();
                break;
            case ResourceType::OTHER:
                if (info.genericResource) {
                    IUnknown* unknown = static_cast(info.genericResource);
                    unknown->Release();
                }
                break;
        }
        resourceInfoMap.erase(it);
    }

    uint64_t getCurrentTimestamp() {
        return std::chrono::duration_cast(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
    }

    void defragmentMemory() {
        std::vector& blocks = dedicatedVRAM.blocks;
        
        // Sort blocks by address
        std::sort(blocks.begin(), blocks.end(), 
            [](const MemoryBlock& a, const MemoryBlock& b) {
                return a.address < b.address;
            });

        // Merge adjacent free blocks
        for (size_t i = 0; i < blocks.size() - 1; i++) {
            if (blocks[i].isFree && blocks[i + 1].isFree) {
                blocks[i].size += blocks[i + 1].size;
                blocks.erase(blocks.begin() + i + 1);
                i--;
            }
        }

        // Compact used blocks to the beginning
        size_t writeIndex = 0;
        for (size_t readIndex = 0; readIndex < blocks.size(); readIndex++) {
            if (!blocks[readIndex].isFree) {
                if (writeIndex != readIndex) {
                    // Move block contents
                    std::memmove((char*)blocks[writeIndex].address, 
                               blocks[readIndex].address,
                               blocks[readIndex].size);
                    // Update block metadata
                    blocks[writeIndex] = blocks[readIndex];
                    blocks[writeIndex].address = (char*)blocks[writeIndex].address;
                }
                writeIndex++;
            }
        }
    }

    void compactResources() {
        std::vector activeResources;
        
        // Collect active resources
        for (const auto& resource : resources) {
            if (resource.isInUse || resource.isPriority) {
                activeResources.push_back(resource);
            } else {
                releaseResource(resource.resource);
                currentCacheSize -= resource.size;
            }
        }

        // Reorganize memory
        resources = activeResources;
        defragmentMemory();
    }

    void prioritizeGameTextures() {
        // Sort resources by priority and usage
        std::sort(resources.begin(), resources.end(),
            [](const ResourceTracker& a, const ResourceTracker& b) {
                if (a.isPriority != b.isPriority) return a.isPriority > b.isPriority;
                if (a.isInUse != b.isInUse) return a.isInUse > b.isInUse;
                return a.lastUseTime > b.lastUseTime;
            });

        // Move high priority resources to dedicated VRAM
        for (auto& resource : resources) {
            if (resource.isPriority && resource.isInUse) {
                auto it = resourceInfoMap.find(resource.resource);
                if (it != resourceInfoMap.end() && it->second.type == ResourceType::TEXTURE) {
                    moveResourceToDedicatedVRAM(resource.resource);
                }
            }
        }
    }

    void moveToSharedMemory(const std::vector& resourceList) {
        for (void* resource : resourceList) {
            auto it = resourceInfoMap.find(resource);
            if (it == resourceInfoMap.end()) continue;

            // Create a new allocation in shared memory
            void* newLocation = allocateInSharedMemory(it->second.textureData.texture);
            if (!newLocation) continue;

            // Copy resource data
            D3D11_MAPPED_SUBRESOURCE mappedResource;
            ID3D11DeviceContext* context = nullptr;
            it->second.textureData.texture->GetDevice()->GetImmediateContext(&context);
            
            if (SUCCEEDED(context->Map(it->second.textureData.texture, 0, 
                                     D3D11_MAP_READ, 0, &mappedResource))) {
                std::memcpy(newLocation, mappedResource.pData, 
                           calculateResourceSize(it->second.textureData.texture));
                context->Unmap(it->second.textureData.texture, 0);
            }
            
            context->Release();
            updateResourceLocation(resource, newLocation);
        }
    }

    std::vector getLowPriorityResources() {
        std::vector lowPriorityList;
        
        for (const auto& resource : resources) {
            if (!resource.isPriority && !resource.isInUse &&
                (getCurrentTimestamp() - resource.lastUseTime) > RESOURCE_TIMEOUT) {
                lowPriorityList.push_back(resource.resource);
            }
        }
        
        // Sort by last use time (oldest first)
        std::sort(lowPriorityList.begin(), lowPriorityList.end(),
            [this](void* a, void* b) {
                auto itA = std::find_if(resources.begin(), resources.end(),
                    [a](const ResourceTracker& r) { return r.resource == a; });
                auto itB = std::find_if(resources.begin(), resources.end(),
                    [b](const ResourceTracker& r) { return r.resource == b; });
                return itA->lastUseTime < itB->lastUseTime;
            });
        
        return lowPriorityList;
    }

    void reserveMemoryBlock(size_t size) {
        // Find a suitable free block
        auto it = std::find_if(dedicatedVRAM.blocks.begin(), dedicatedVRAM.blocks.end(),
            [size](const MemoryBlock& block) {
                return block.isFree && block.size >= size;
            });

        if (it != dedicatedVRAM.blocks.end()) {
            // Split block if necessary
            if (it->size > size + sizeof(MemoryBlock)) {
                MemoryBlock newBlock;
                newBlock.address = static_cast(it->address) + size;
                newBlock.size = it->size - size;
                newBlock.isFree = true;
                
                it->size = size;
                dedicatedVRAM.blocks.insert(it + 1, newBlock);
            }
            
            it->isFree = false;
            dedicatedVRAM.usedSize += size;
        } else {
            // Trigger defragmentation and retry
            defragmentMemory();
            reserveMemoryBlock(size);
        }
    }

    void markResourceUsed(void* resource) {
        auto it = std::find_if(resources.begin(), resources.end(),
            [resource](const ResourceTracker& r) {
                return r.resource == resource;
            });

        if (it != resources.end()) {
            it->lastUseTime = getCurrentTimestamp();
            it->isInUse = true;
        }
    }

    void* allocateInSharedMemory(ID3D11Resource* resource) {
        D3D11_RESOURCE_DIMENSION type;
        resource->GetType(&type);

        // Get resource description
        D3D11_TEXTURE2D_DESC desc;
        if (type == D3D11_RESOURCE_DIMENSION_TEXTURE2D) {
            ID3D11Texture2D* texture = static_cast(resource);
            texture->GetDesc(&desc);
        }

        // Calculate required size
        size_t size = calculateResourceSize(resource);
        
        // Find free block in shared memory
        auto it = std::find_if(sharedVRAM.blocks.begin(), sharedVRAM.blocks.end(),
            [size](const MemoryBlock& block) {
                return block.isFree && block.size >= size;
            });

        if (it != sharedVRAM.blocks.end()) {
            it->isFree = false;
            sharedVRAM.usedSize += size;
            return it->address;
        }

        // If no suitable block found, allocate new shared memory
        void* newBlock = VirtualAlloc(nullptr, size, 
                                     MEM_COMMIT | MEM_RESERVE | MEM_WRITE_WATCH,
                                     PAGE_READWRITE);
        
        if (newBlock) {
            MemoryBlock block = {
                newBlock,
                size,
                false
            };
            sharedVRAM.blocks.push_back(block);
            sharedVRAM.usedSize += size;
        }

        return newBlock;
    }

    void updateResourceLocation(void* oldLocation, void* newLocation) {
        // Find resource info
        auto it = resourceInfoMap.begin();
        while (it != resourceInfoMap.end()) {
            if (it->first == oldLocation) {
                // Create new mapping
                ResourceInfo info = it->second;
                resourceInfoMap[newLocation] = info;
                
                // Update resource pointers based on type
                switch(info.type) {
                    case ResourceType::TEXTURE:
                        info.textureData.texture = static_cast(newLocation);
                        break;
                    case ResourceType::VERTEX_BUFFER:
                    case ResourceType::INDEX_BUFFER:
                        info.bufferData.buffer = static_cast(newLocation);
                        break;
                    case ResourceType::SHADER:
                        info.shaderData.shader = static_cast(newLocation);
                        break;
                    case ResourceType::RENDER_TARGET:
                        info.renderTargetData.texture = static_cast(newLocation);
                        break;
                    case ResourceType::OTHER:
                        info.genericResource = newLocation;
                        break;
                }
                
                // Remove old mapping
                resourceInfoMap.erase(it);
                break;
            }
            ++it;
        }

        // Update resource trackers
        for (auto& tracker : resources) {
            if (tracker.resource == oldLocation) {
                tracker.resource = newLocation;
            }
        }
    }

    size_t calculateResourceSize(ID3D11Resource* resource) {
        D3D11_RESOURCE_DIMENSION type;
        resource->GetType(&type);
        
        switch(type) {
            case D3D11_RESOURCE_DIMENSION_TEXTURE2D: {
                ID3D11Texture2D* texture = static_cast(resource);
                D3D11_TEXTURE2D_DESC desc;
                texture->GetDesc(&desc);
                
                UINT bytesPerPixel;
                switch(desc.Format) {
                    case DXGI_FORMAT_R8G8B8A8_UNORM:
                    case DXGI_FORMAT_R8G8B8A8_UINT:
                        bytesPerPixel = 4;
                        break;
                    case DXGI_FORMAT_R16G16B16A16_FLOAT:
                        bytesPerPixel = 8;
                        break;
                    case DXGI_FORMAT_R32G32B32A32_FLOAT:
                        bytesPerPixel = 16;
                        break;
                    default:
                        bytesPerPixel = 4; // Default to 32-bit
                }
                
                return desc.Width * desc.Height * bytesPerPixel * desc.ArraySize;
            }
            
            case D3D11_RESOURCE_DIMENSION_BUFFER: {
                ID3D11Buffer* buffer = static_cast(resource);
                D3D11_BUFFER_DESC desc;
                buffer->GetDesc(&desc);
                return desc.ByteWidth;
            }
            
            default:
                return 0;
        }
    }

    void moveResourceToDedicatedVRAM(void* resource) {
        auto it = resourceInfoMap.find(resource);
        if (it == resourceInfoMap.end()) return;

        ID3D11Device* device;
        if (it->second.type == ResourceType::TEXTURE) {
            it->second.textureData.texture->GetDevice(&device);
        } else {
            return; // Only handle textures for now
        }

        // Calculate required size
        size_t size = calculateResourceSize(static_cast(resource));

        // Find or create space in dedicated VRAM
        void* newLocation = nullptr;
        D3D11_TEXTURE2D_DESC desc;
        it->second.textureData.texture->GetDesc(&desc);
        
        // Set usage to allow GPU write
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags |= D3D11_RESOURCE_MISC_SHARED;

        ID3D11Texture2D* newTexture;
        if (SUCCEEDED(device->CreateTexture2D(&desc, nullptr, &newTexture))) {
            // Copy resource data
            ID3D11DeviceContext* context;
            device->GetImmediateContext(&context);
            context->CopyResource(newTexture, it->second.textureData.texture);
            
            // Update resource mapping
            newLocation = newTexture;
            updateResourceLocation(resource, newLocation);
            
            context->Release();
        }

        device->Release();
    }
};

#endif