#pragma once
#include 
#include 
#include 
#include 

namespace Win32Compat {

class ControlsManager {
private:
    static std::unique_ptr instance;
    std::unordered_map controls;
    
    ControlsManager() = default;

public:
    static ControlsManager* getInstance() {
        if (!instance) {
            instance = std::unique_ptr(new ControlsManager());
        }
        return instance.get();
    }

    HWND createButton(const ButtonParams& params) {
        ControlInfo info;
        info.type = CTRL_BUTTON;
        info.handle = generateControlHandle();
        info.parent = params.parent;
        info.style = params.style;
        
        controls[info.handle] = info;
        return info.handle;
    }

    HWND createEdit(const EditParams& params) {
        ControlInfo info;
        info.type = CTRL_EDIT;
        info.handle = generateControlHandle();
        info.parent = params.parent;
        info.style = params.style;
        
        controls[info.handle] = info;
        return info.handle;
    }

    HWND createListBox(const ListBoxParams& params) {
        ControlInfo info;
        info.type = CTRL_LISTBOX;
        info.handle = generateControlHandle();
        info.parent = params.parent;
        info.style = params.style;
        
        controls[info.handle] = info;
        return info.handle;
    }

    bool destroyControl(HWND handle) {
        auto it = controls.find(handle);
        if (it != controls.end()) {
            controls.erase(it);
            return true;
        }
        return false;
    }

private:
    HWND generateControlHandle() {
        static HWND nextHandle = (HWND)5000;
        return nextHandle++;
    }

    enum ControlType {
        CTRL_BUTTON,
        CTRL_EDIT,
        CTRL_LISTBOX
    };

    struct ControlInfo {
        HWND handle;
        HWND parent;
        ControlType type;
        DWORD style;
    };
};

} // namespace Win32Compat