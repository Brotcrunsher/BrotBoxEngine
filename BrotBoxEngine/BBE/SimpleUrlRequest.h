#pragma once

#ifdef BBE_ADD_CURL
#include <future>
#include <optional>

#include "../BBE/String.h"
#include "../BBE/List.h"

namespace bbe
{
	namespace simpleUrlRequest
	{
		struct UrlRequestResult
		{
			bbe::List<char> dataContainer;
			long responseCode;
		};

		UrlRequestResult urlRequest(const bbe::String& url, bool addTrailingNul = true, bool verbose = false);
		std::future<UrlRequestResult> urlRequestAsync(const bbe::String& url, bool addTrailingNul = true, bool verbose = false);

		std::optional<bbe::List<char>> decryptXChaCha(const bbe::List<char>& data, const String& pathToKeyFile, bool addTrailingNul = true);

#ifdef _WIN32
		enum class SocketRequestXChaChaCode
		{
			SUCCESS,
			HOST_NOT_REACHABLE,
			FAILED_TO_DECRYPT
		};
		struct SocketRequestXChaChaRet
		{
			SocketRequestXChaChaCode code = SocketRequestXChaChaCode::SUCCESS;
			bbe::List<char> dataContainer;
		};
		SocketRequestXChaChaRet socketRequestXChaCha(const bbe::String& url, uint16_t port, const String& pathToKeyFile, bool addTrailingNul = true, bool verbose = false);
		std::future<SocketRequestXChaChaRet> socketRequestXChaChaAsync(bbe::String /*copy*/ url, uint16_t port, const String& pathToKeyFile, bool addTrailingNul = true, bool verbose = false);
#endif
	}
}
#endif
