#pragma once
#include 
#include "../com/com_manager.hpp"

namespace Win32Compat {
class Ole32Wrapper {
private:
    ComManager* comManager;

public:
    Ole32Wrapper() {
        comManager = ComManager::getInstance();
    }

    HRESULT CoInitialize(LPVOID pvReserved) {
        return comManager->initialize();
    }

    void CoUninitialize() {
        comManager->uninitialize();
    }
    
    HRESULT CoCreateInstance(
        REFCLSID rclsid,
        LPUNKNOWN pUnkOuter,
        DWORD dwClsContext,
        REFIID riid,
        LPVOID* ppv
    ) {
        return comManager->createInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);
    }
};
} // namespace Win32Compat