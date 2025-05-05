#pragma once
#include 
#include "../filesystem/fs_manager.hpp"
#include "../security/security_manager.hpp"

namespace Win32Compat {

class Shell32Wrapper {
private:
    FileSystemManager* fsManager;
    SecurityManager* securityManager;

public:
    Shell32Wrapper() {
        fsManager = FileSystemManager::getInstance();
        securityManager = SecurityManager::getInstance();
    }

    HRESULT SHGetFolderPathW(
        HWND hwnd,
        int csidl,
        HANDLE hToken,
        DWORD dwFlags,
        LPWSTR pszPath
    ) {
        return fsManager->getSpecialFolderPath(convertCSIDL(csidl), pszPath);
    }

    HRESULT SHGetKnownFolderPath(
        REFKNOWNFOLDERID rfid,
        DWORD dwFlags,
        HANDLE hToken,
        PWSTR* ppszPath
    ) {
        return fsManager->getKnownFolderPath(convertFolderID(rfid), ppszPath);
    }
};

} // namespace Win32Compat