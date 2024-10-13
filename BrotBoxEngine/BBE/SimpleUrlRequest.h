#pragma once

#ifdef BBE_ADD_CURL
#include <future>
#include <optional>
#include <map>

#include "../BBE/String.h"
#include "../BBE/List.h"
#include "../BBE/ByteBuffer.h"

namespace bbe
{
	namespace simpleUrlRequest
	{
		struct UrlRequestResult
		{
			bbe::List<char> dataContainer;
			long responseCode;
		};

		UrlRequestResult urlRequest(const bbe::String& url, const bbe::List<bbe::String>& headerFields = {}, const bbe::String& postData = "", bool addTrailingNul = true, bool verbose = false);
		std::future<UrlRequestResult> urlRequestAsync(const bbe::String& url, const bbe::List<bbe::String>& headerFields = {}, const bbe::String& postData = "", bool addTrailingNul = true, bool verbose = false);

		bbe::simpleUrlRequest::UrlRequestResult urlFile(
			const bbe::String& url,
			const bbe::List<bbe::String>& headerFields = {},
			const std::map<bbe::String, bbe::String>& formFields = {},
			const bbe::ByteBuffer* fileData = nullptr,
			const bbe::String& fileFieldName = "",
			const bbe::String& fileName = "",
			bool addTrailingNul = true,
			bool verbose = false);
		std::future<UrlRequestResult> urlFileAsync(
			const bbe::String& url,
			const bbe::List<bbe::String>& headerFields = {},
			const std::map<bbe::String, bbe::String>& formFields = {},
			const bbe::ByteBuffer* fileData = nullptr,
			const bbe::String& fileFieldName = "",
			const bbe::String& fileName = "",
			bool addTrailingNul = true,
			bool verbose = false);
		
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
		
		bbe::List<bbe::String> resolveDomain(const bbe::String& domain);
		bbe::List<bbe::String> resolveDomains(const bbe::List<bbe::String>& domains);
		bbe::String firewallBlockString(const bbe::List<bbe::String>& domains);
		void firewallBlockDomains(const bbe::String& ruleName, const bbe::List<bbe::String>& domains);
#endif
	}
}
#endif
