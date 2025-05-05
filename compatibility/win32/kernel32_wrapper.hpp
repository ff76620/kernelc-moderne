#pragma once
#include 
#include "../memory/vmm.hpp"
#include "../process/process.hpp"

namespace Win32Compat {

class Kernel32Wrapper {
private:
    MemoryManager* memoryManager;
    ProcessManager* processManager;

public:
    Kernel32Wrapper() {
        memoryManager = MemoryManager::getInstance();
        processManager = ProcessManager::getInstance();
    }

    HANDLE CreateProcessW(
        LPCWSTR lpApplicationName,
        LPWSTR lpCommandLine,
        LPSECURITY_ATTRIBUTES lpProcessAttributes,
        LPSECURITY_ATTRIBUTES lpThreadAttributes,
        BOOL bInheritHandles,
        DWORD dwCreationFlags,
        LPVOID lpEnvironment,
        LPCWSTR lpCurrentDirectory,
        LPSTARTUPINFOW lpStartupInfo,
        LPPROCESS_INFORMATION lpProcessInformation
    ) {
        ProcessCreationParams params;
        params.applicationPath = convertToNativePath(lpApplicationName);
        params.commandLine = convertCommandLine(lpCommandLine);
        params.inheritHandles = bInheritHandles;
        
        return processManager->createProcess(params);
    }

    LPVOID VirtualAlloc(
        LPVOID lpAddress,
        SIZE_T dwSize,
        DWORD flAllocationType,
        DWORD flProtect
    ) {
        MemoryProtectionFlags protFlags = convertProtectionFlags(flProtect);
        return memoryManager->allocateMemory(lpAddress, dwSize, protFlags);
    }

    BOOL VirtualFree(
        LPVOID lpAddress,
        SIZE_T dwSize,
        DWORD dwFreeType
    ) {
        return memoryManager->freeMemory(lpAddress, dwSize);
    }

    HANDLE CreateFileW(
        LPCWSTR lpFileName,
        DWORD dwDesiredAccess,
        DWORD dwShareMode,
        LPSECURITY_ATTRIBUTES lpSecurityAttributes,
        DWORD dwCreationDisposition,
        DWORD dwFlagsAndAttributes,
        HANDLE hTemplateFile
    ) {
        FileSystemParams params;
        params.path = convertToNativePath(lpFileName);
        params.access = convertAccessFlags(dwDesiredAccess);
        params.shareMode = convertShareMode(dwShareMode);
        
        return fileSystem->createFile(params);
    }
};

} // namespace Win32Compat