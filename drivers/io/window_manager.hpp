#pragma once
#include 
#include 
#include "../core/kernel.hpp"
#include "../drivers/input/input_manager.hpp"

namespace Win32Compat {

class WindowManager {
private:
    static std::unique_ptr instance;
    std::unordered_map windows;
    
    WindowManager() = default;

public:
    static WindowManager* getInstance() {
        if (!instance) {
            instance = std::unique_ptr(new WindowManager());
        }
        return instance.get();
    }

    HWND createWindow(const WindowCreationParams& params) {
        WindowInfo info;
        info.handle = generateWindowHandle();
        info.title = params.title;
        info.style = params.style;
        info.x = params.x;
        info.y = params.y;
        info.width = params.width;
        info.height = params.height;
        
        windows[info.handle] = info;
        return info.handle;
    }

    bool showWindow(HWND hWnd, int cmdShow) {
        auto it = windows.find(hWnd);
        if (it != windows.end()) {
            it->second.visible = (cmdShow == SW_SHOW);
            return true;
        }
        return false;
    }

    LRESULT dispatchMessage(const MSG* msg) {
        auto it = windows.find(msg->hwnd);
        if (it != windows.end()) {
            return windowProc(msg->hwnd, msg->message, msg->wParam, msg->lParam);
        }
        return 0;
    }

private:
    HWND generateWindowHandle() {
        static HWND nextHandle = (HWND)1000;
        return nextHandle++;
    }

    LRESULT windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

} // namespace Win32Compat