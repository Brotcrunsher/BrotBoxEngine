#pragma once

#ifdef WIN32
#include "../BBE/List.h"
#include "../BBE/String.h"
#include <cstdint>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>

namespace bbe
{
	class Socket
	{
	private:
		SOCKET nativeSocket = INVALID_SOCKET;
	public:
		Socket(bbe::String /*copy*/ url, uint16_t port);
		~Socket();

		Socket(const Socket&) = delete;
		Socket(Socket&&) = delete;

		bool established() const;

		bbe::List<char> drain();
	};
}
#endif
