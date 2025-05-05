#pragma once
#include 
#include 

namespace Win32Compat {

class WebView {
private:
    static std::unique_ptr instance;
    
    WebView() = default;

public:
    static WebView* getInstance() {
        if (!instance) {
            instance = std::unique_ptr(new WebView());
        }
        return instance.get();
    }

    bool initialize(HWND parentWindow) {
        // Initialize web view control
        return true;
    }

    bool navigate(const std::wstring& url) {
        // Navigate to URL
        return true;
    }
    
    bool loadHTML(const std::string& html) {
        // Load HTML content directly
        return true;
    }

    bool evaluateScript(const std::string& script, std::string& result) {
        // Execute JavaScript and get result
        return true;
    }

    void cleanup() {
        // Release resources
    }
};

} // namespace Win32Compat