#ifdef WIN32
#include "BBE/Socket.h"
#include <WS2tcpip.h>

bbe::Socket::Socket(bbe::String /*copy*/ url, uint16_t port)
{
    nativeSocket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (nativeSocket == INVALID_SOCKET)
    {
        bbe::Crash(bbe::Error::IllegalState);
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    struct addrinfo hints = { 0 };
    hints.ai_family = AF_INET; // Only IPv4
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    struct addrinfo* result = nullptr;
    int err = getaddrinfo(url.getRaw(), nullptr, &hints, &result);
    if (err != 0 || result == nullptr)
    {
        if (url == "localhost") url = "127.0.0.1";
        addr.sin_addr.s_addr = inet_pton(AF_INET, url.getRaw(), &addr.sin_addr);
    }
    else
    {
        addr.sin_addr = ((struct sockaddr_in*)result->ai_addr)->sin_addr;
        freeaddrinfo(result);
    }

    if (connect(nativeSocket, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        closesocket(nativeSocket);
        bbe::Crash(bbe::Error::IllegalState);
    }
}

bbe::Socket::~Socket()
{
    if (nativeSocket != INVALID_SOCKET && closesocket(nativeSocket) == SOCKET_ERROR)
    {
        bbe::Crash(bbe::Error::IllegalState);
    }
}

bool bbe::Socket::established() const
{
    return nativeSocket != INVALID_SOCKET;
}

bbe::List<char> bbe::Socket::drain()
{
    bbe::List<char> retVal;
    char buffer[4096];
    while (true)
    {
        int recvLength = recv(nativeSocket, buffer, sizeof(buffer), 0);
        if (recvLength <= 0)
        {
            break;
        }
        retVal.addArray(buffer, recvLength);
    }
    return retVal;
}

#endif
