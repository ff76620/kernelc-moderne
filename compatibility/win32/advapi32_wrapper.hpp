#pragma once
#include 
#include "../security/security_manager.hpp"
#include "../process/process.hpp"

namespace Win32Compat {

class Advapi32Wrapper {
private:
    SecurityManager* securityManager;
    ProcessManager* processManager;

public:
    Advapi32Wrapper() {
        securityManager = SecurityManager::getInstance();
        processManager = ProcessManager::getInstance();
    }

    BOOL LogonUserW(
        LPCWSTR lpszUsername,
        LPCWSTR lpszDomain,
        LPCWSTR lpszPassword,
        DWORD dwLogonType,
        DWORD dwLogonProvider,
        PHANDLE phToken
    ) {
        UserCredentials creds;
        creds.username = convertToNativeString(lpszUsername);
        creds.domain = convertToNativeString(lpszDomain);
        creds.password = convertToNativeString(lpszPassword);
        
        return securityManager->logonUser(creds, phToken);
    }

    BOOL CreateProcessAsUserW(
        HANDLE hToken,
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
        params.token = hToken;
        params.applicationPath = convertToNativePath(lpApplicationName);
        params.commandLine = convertCommandLine(lpCommandLine);
        
        return processManager->createProcessAsUser(params);
    }
};

} // namespace Win32Compat