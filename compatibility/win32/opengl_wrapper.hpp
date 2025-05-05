#pragma once
#include 
#include "../drivers/graphics/graphics_device.hpp"

namespace Win32Compat {
class OpenGLWrapper {
private:
    GraphicsDevice* device;

public:
    OpenGLWrapper() {
        device = GraphicsDevice::getInstance();
    }

    BOOL wglMakeCurrent(HDC hdc, HGLRC hglrc) {
        return device->makeOpenGLContextCurrent(hdc, hglrc);
    }

    HGLRC wglCreateContext(HDC hdc) {
        return device->createOpenGLContext(hdc);
    }

    BOOL wglDeleteContext(HGLRC hglrc) {
        return device->deleteOpenGLContext(hglrc);
    }
};
} // namespace Win32Compat