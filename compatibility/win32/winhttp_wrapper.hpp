#pragma once
#include 
#include "../network/network_stack.hpp"

namespace Win32Compat {
class WinHTTPWrapper {
private:
    NetworkStack* network;

public:
    WinHTTPWrapper() {
        network = NetworkStack::getInstance();
    }

    HINTERNET WinHttpOpen(
        LPCWSTR pszAgentW,
        DWORD dwAccessType,
        LPCWSTR pszProxyW,
        LPCWSTR pszProxyBypassW,
        DWORD dwFlags
    ) {
        ConnectionParams params;
        params.userAgent = convertToNativeString(pszAgentW);
        params.accessType = dwAccessType;
        params.proxy = convertToNativeString(pszProxyW);
        params.proxyBypass = convertToNativeString(pszProxyBypassW);
        params.flags = dwFlags;
        
        return network->openConnection(params);
    }

    HINTERNET WinHttpConnect(
        HINTERNET hSession,
        LPCWSTR pswzServerName,
        INTERNET_PORT nServerPort,
        DWORD dwReserved
    ) {
        ConnectParams params;
        params.session = hSession;
        params.server = convertToNativeString(pswzServerName);
        params.port = nServerPort;
        
        return network->connect(params);
    }

    HINTERNET WinHttpOpenRequest(
        HINTERNET hConnect,
        LPCWSTR pwszVerb,
        LPCWSTR pwszObjectName,
        LPCWSTR pwszVersion,
        LPCWSTR pwszReferrer,
        LPCWSTR* ppwszAcceptTypes,
        DWORD dwFlags
    ) {
        RequestParams params;
        params.connection = hConnect;
        params.verb = convertToNativeString(pwszVerb);
        params.object = convertToNativeString(pwszObjectName);
        params.version = convertToNativeString(pwszVersion);
        params.referrer = convertToNativeString(pwszReferrer);
        params.acceptTypes = convertAcceptTypes(ppwszAcceptTypes);
        params.flags = dwFlags;
        
        return network->openRequest(params);
    }

    BOOL WinHttpSendRequest(
        HINTERNET hRequest,
        LPCWSTR lpszHeaders,
        DWORD dwHeadersLength,
        LPVOID lpOptional,
        DWORD dwOptionalLength,
        DWORD dwTotalLength,
        DWORD_PTR dwContext
    ) {
        SendRequestParams params;
        params.request = hRequest;
        params.headers = convertToNativeString(lpszHeaders);
        params.headerLength = dwHeadersLength;
        params.optional = lpOptional;
        params.optionalLength = dwOptionalLength;
        params.totalLength = dwTotalLength;
        params.context = dwContext;
        
        return network->sendRequest(params);
    }
};
} // namespace Win32Compat