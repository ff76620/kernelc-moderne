#pragma once
#include 
#include "../drivers/input/input_manager.hpp"
#include "../drivers/io/window_manager.hpp"

namespace Win32Compat {

class User32Wrapper {
private:
    InputManager* inputManager;
    WindowManager* windowManager;

public:
    User32Wrapper() {
        inputManager = InputManager::getInstance();
        windowManager = WindowManager::getInstance();
    }

    HWND CreateWindowExW(
        DWORD dwExStyle,
        LPCWSTR lpClassName,
        LPCWSTR lpWindowName,
        DWORD dwStyle,
        int X, int Y,
        int nWidth, int nHeight,
        HWND hWndParent,
        HMENU hMenu,
        HINSTANCE hInstance,
        LPVOID lpParam
    ) {
        WindowCreationParams params;
        params.title = convertToNativeString(lpWindowName);
        params.style = convertWindowStyle(dwStyle);
        params.x = X;
        params.y = Y;
        params.width = nWidth;
        params.height = nHeight;
        
        return windowManager->createWindow(params);
    }

    BOOL ShowWindow(
        HWND hWnd,
        int nCmdShow
    ) {
        return windowManager->showWindow(hWnd, convertShowCommand(nCmdShow));
    }

    BOOL GetMessageW(
        LPMSG lpMsg,
        HWND hWnd,
        UINT wMsgFilterMin,
        UINT wMsgFilterMax
    ) {
        return inputManager->getMessage(lpMsg, hWnd);
    }

    BOOL TranslateMessage(
        const MSG *lpMsg
    ) {
        return inputManager->translateMessage(lpMsg);
    }

    LRESULT DispatchMessageW(
        const MSG *lpMsg
    ) {
        return windowManager->dispatchMessage(lpMsg);
    }
};

} // namespace Win32Compat