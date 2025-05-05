#pragma once
#include 
#include 

namespace Win32Compat {

class DialogManager {
private:
    static std::unique_ptr instance;
    
    DialogManager() = default;

public:
    static DialogManager* getInstance() {
        if (!instance) {
            instance = std::unique_ptr(new DialogManager());
        }
        return instance.get();
    }

    bool showOpenDialog(const DialogParams& params, LPWSTR outFile) {
        // Validate parameters
        if (!validateDialogParams(params)) {
            return false;
        }

        // Show open dialog UI
        bool result = displayOpenDialog(params);
        if (result) {
            // Copy selected file path to output buffer
            wcscpy_s(outFile, MAX_PATH, selectedFilePath.c_str());
        }
        
        return result;
    }

    bool showSaveDialog(const DialogParams& params, LPWSTR outFile) {
        if (!validateDialogParams(params)) {
            return false;
        }

        bool result = displaySaveDialog(params);
        if (result) {
            wcscpy_s(outFile, MAX_PATH, selectedFilePath.c_str());
        }

        return result;
    }

private:
    std::wstring selectedFilePath;

    bool validateDialogParams(const DialogParams& params);
    bool displayOpenDialog(const DialogParams& params);
    bool displaySaveDialog(const DialogParams& params);
};

} // namespace Win32Compat