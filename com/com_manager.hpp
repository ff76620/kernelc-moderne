#pragma once
#include 
#include 
#include 

namespace Win32Compat {

class ComManager {
private:
    static std::unique_ptr instance;
    bool initialized;
    std::unordered_map threadInitMap;
    
    ComManager() : initialized(false) {}

public:
    static ComManager* getInstance() {
        if (!instance) {
            instance = std::unique_ptr(new ComManager());
        }
        return instance.get();
    }

    HRESULT initialize() {
        if (initialized) {
            return S_FALSE;
        }

        // Initialize COM for current thread
        HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        if (SUCCEEDED(hr)) {
            threadInitMap[GetCurrentThreadId()] = true;
            initialized = true;
        }
        return hr;
    }

    void uninitialize() {
        DWORD threadId = GetCurrentThreadId();
        if (threadInitMap[threadId]) {
            CoUninitialize();
            threadInitMap[threadId] = false;
        }
    }
    
    HRESULT createInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter, 
                          DWORD dwClsContext, REFIID riid, LPVOID* ppv) {
        if (!initialized) {
            return CO_E_NOTINITIALIZED;
        }
        return CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);
    }

    HRESULT allocateString(const OLECHAR* sz) {
        return SysAllocString(sz);
    }

    void freeString(BSTR bstr) {
        SysFreeString(bstr);
    }
};

} // namespace Win32Compat