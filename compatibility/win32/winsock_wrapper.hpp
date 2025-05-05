#pragma once
#include 
#include "../network/network_stack.hpp"

namespace Win32Compat {
  class WinSockWrapper {
  private:
    NetworkStack* networkStack;

  public:
    WinSockWrapper() {
      networkStack = NetworkStack::getInstance(); 
    }

    SOCKET socket(int af, int type, int protocol) {
      SocketParams params;
      params.family = convertAddressFamily(af);
      params.type = convertSocketType(type);
      params.protocol = protocol;
      return networkStack->createSocket(params);
    }

    int connect(SOCKET s, const sockaddr* name, int namelen) {
      return networkStack->connectSocket(s, convertAddress(name), namelen);
    }

    int send(SOCKET s, const char* buf, int len, int flags) {
      return networkStack->sendData(s, buf, len, convertFlags(flags));
    }

    int recv(SOCKET s, char* buf, int len, int flags) {
      return networkStack->receiveData(s, buf, len, convertFlags(flags));
    }
  };
}