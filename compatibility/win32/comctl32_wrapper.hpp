#pragma once
#include 
#include "../ui/controls_manager.hpp"
#include "../ui/window_manager.hpp"
        
namespace Win32Compat {
class ComCtl32Wrapper {
private:
    ControlsManager* controlsManager;
    WindowManager* windowManager;

public:
    ComCtl32Wrapper() {
        controlsManager = ControlsManager::getInstance();
        windowManager = WindowManager::getInstance();
    }

    HWND CreateToolbarEx(
        HWND hwnd,
        DWORD ws, 
        UINT wID,
        int nBitmaps,
        HINSTANCE hBMInst,
        UINT_PTR wBMID,
        LPCTBBUTTON lpButtons,
        int iNumButtons,
        int dxButton,
        int dyButton,
        int dxBitmap,
        int dyBitmap,
        UINT uStructSize
    ) {
        ToolbarParams params;
        params.parentWindow = hwnd;
        params.style = ws;
        params.id = wID;
        params.buttonCount = iNumButtons;
        params.buttons = convertToolbarButtons(lpButtons, iNumButtons);
        params.buttonSize = {dxButton, dyButton};
        params.bitmapSize = {dxBitmap, dyBitmap};
        
        return controlsManager->createToolbar(params);
    }

    HWND CreateStatusWindow(
        LONG style,
        LPCTSTR lpszText,
        HWND hwndParent,
        UINT wID
    ) {
        StatusBarParams params;
        params.style = style;
        params.text = convertToNativeString(lpszText);
        params.parent = hwndParent;
        params.id = wID;

        return controlsManager->createStatusBar(params);
    }

    HWND CreateUpDownControl(
        DWORD dwStyle,
        int x, int y, int cx, int cy,
        HWND hParent,
        int nID,
        HINSTANCE hInst,
        HWND hBuddy,
        int nUpper,
        int nLower,
        int nPos
    ) {
        UpDownParams params;
        params.style = dwStyle;
        params.position = {x, y};
        params.size = {cx, cy};
        params.parent = hParent;
        params.id = nID;
        params.buddyWindow = hBuddy;
        params.range = {nLower, nUpper};
        params.initialPos = nPos;

        return controlsManager->createUpDownControl(params);
    }

    HWND CreateProgressBar(
        HWND hwndParent,
        int x, int y, int cx, int cy,
        DWORD dwStyle
    ) {
        ProgressParams params;
        params.parent = hwndParent;
        params.position = {x, y}; 
        params.size = {cx, cy};
        params.style = dwStyle;

        return controlsManager->createProgressBar(params);
    }

    BOOL InitCommonControlsEx(
        const INITCOMMONCONTROLSEX* picce
    ) {
        return controlsManager->initializeControls(picce->dwICC);
    }

    int ImageList_Create(
        int cx, int cy,
        UINT flags,
        int cInitial,
        int cGrow
    ) {
        ImageListParams params;
        params.iconSize = {cx, cy};
        params.flags = flags;
        params.initialCount = cInitial;
        params.growCount = cGrow;

        return controlsManager->createImageList(params);
    }

    BOOL ImageList_Destroy(int himl) {
        return controlsManager->destroyImageList(himl);
    }

    int ImageList_Add(
        int himl,
        HBITMAP hbmImage,
        HBITMAP hbmMask
    ) {
        return controlsManager->addImageToList(himl, hbmImage, hbmMask);
    }

    BOOL TreeView_SetImageList(
        HWND hwnd,
        int himl,
        int iImage
    ) {
        return controlsManager->setTreeViewImageList(hwnd, himl, iImage);
    }
};
} // namespace Win32Compat