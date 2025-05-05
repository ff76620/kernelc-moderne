#pragma once
#include 
#include "../com/com_manager.hpp"

namespace Win32Compat {
class OleAut32Wrapper {
private:
    ComManager* comManager;

public:
    OleAut32Wrapper() {
        comManager = ComManager::getInstance();
    }

    HRESULT SysAllocString(const OLECHAR* sz) {
        return comManager->allocateString(sz);
    }

    void SysFreeString(BSTR bstr) {
        comManager->freeString(bstr);
    }
};
} // namespace Win32Compat
      