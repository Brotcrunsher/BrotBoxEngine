#ifdef WIN32
#include "BBE/Socket.h"

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

	hostent* hent = gethostbyname(url.getRaw());
	if (hent && hent->h_addrtype == AF_INET && hent->h_length == 4)
	{
		// TODO: IPv6
		addr.sin_addr.s_addr = *(u_long*)hent->h_addr_list[0];
	}
	else
	{
		if (url == "localhost") url = "127.0.0.1";
		addr.sin_addr.s_addr = inet_addr(url.getRaw());
	}

	connect(nativeSocket, (SOCKADDR*)&addr, sizeof(addr));
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
