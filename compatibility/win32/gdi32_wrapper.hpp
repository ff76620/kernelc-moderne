#pragma once
#include 
#include "../drivers/graphics/graphics_manager.hpp"

namespace Win32Compat {

class GDI32Wrapper {
private:
    GraphicsManager* graphicsManager;

public:
    GDI32Wrapper() {
        graphicsManager = GraphicsManager::getInstance();
    }

    HDC CreateCompatibleDC(
        HDC hdc
    ) {
        return graphicsManager->createDeviceContext();
    }

    HBITMAP CreateCompatibleBitmap(
        HDC hdc,
        int width,
        int height
    ) {
        return graphicsManager->createBitmap(width, height);
    }

    BOOL BitBlt(
        HDC hdc,
        int x,
        int y,
        int cx,
        int cy,
        HDC hdcSrc,
        int x1,
        int y1,
        DWORD rop
    ) {
        BlitParams params;
        params.sourceX = x1;
        params.sourceY = y1;
        params.destX = x;
        params.destY = y;
        params.width = cx;
        params.height = cy;
        
        return graphicsManager->blitSurface(params);
    }

    HBRUSH CreateSolidBrush(
        COLORREF color
    ) {
        return graphicsManager->createBrush(color);
    }

    BOOL Rectangle(
        HDC hdc,
        int left,
        int top,
        int right,
        int bottom
    ) {
        RectParams params;
        params.left = left;
        params.top = top;
        params.right = right;
        params.bottom = bottom;
        
        return graphicsManager->drawRectangle(params);
    }
};

} // namespace Win32Compat