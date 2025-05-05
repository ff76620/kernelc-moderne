#pragma once
#include 
#include "../core/kernel.hpp"
#include "../multimedia/pipeline_manager.hpp"

namespace Win32Compat {

class GraphicsDevice {
private:
    PipelineManager* pipelineManager;
    
    struct Surface {
        void* data;
        int width;
        int height;
        int pitch;
        int bpp;
    };

    std::unordered_map surfaces;

public:
    GraphicsDevice() {
        pipelineManager = PipelineManager::getInstance();
    }

    bool blit(const BlitParams& params) {
        auto srcSurface = surfaces.find(params.sourceId);
        auto dstSurface = surfaces.find(params.destId);
        
        if (srcSurface == surfaces.end() || dstSurface == surfaces.end()) {
            return false;
        }

        return pipelineManager->blitSurface(
            srcSurface->second.data,
            dstSurface->second.data,
            params.sourceX,
            params.sourceY, 
            params.destX,
            params.destY,
            params.width,
            params.height
        );
    }

    bool drawRect(const RectParams& params) {
        auto surface = surfaces.find(params.surfaceId);
        if (surface == surfaces.end()) {
            return false; 
        }

        return pipelineManager->drawRectangle(
            surface->second.data,
            params.left,
            params.top,
            params.right - params.left,
            params.bottom - params.top,
            params.color
        );
    }

    Surface* createSurface(int width, int height, int bpp) {
        Surface surface;
        surface.width = width;
        surface.height = height;
        surface.bpp = bpp;
        surface.pitch = width * (bpp / 8);
        surface.data = malloc(surface.pitch * height);

        int id = generateSurfaceId();
        surfaces[id] = surface;
        return &surfaces[id];
    }

private:
    int generateSurfaceId() {
        static int nextId = 1;
        return nextId++;
    }
};

} // namespace Win32Compat