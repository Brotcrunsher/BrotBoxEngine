#pragma once

#ifdef BBE_ADD_CURL
#include <future>
#include <functional>
#include <optional>
#include <map>
#include <mutex>

#include "nlohmann/json.hpp"

#include "../BBE/String.h"
#include "../BBE/List.h"
#include "../BBE/ByteBuffer.h"
#include "../BBE/Logging.h"
#include "../BBE/Async.h"

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
		
		std::optional<bbe::List<char>> decryptXChaCha(const bbe::List<char>& data, const bbe::ByteBuffer& key, bool addTrailingNul = true);

		template<typename T>
		void jsonElement(T* value, const nlohmann::json& json, const char* jsonPath, int32_t depth = 0)
		{
			const char* nextSlash = strstr(jsonPath, "/");
			bbe::String token;
			if (!nextSlash) token = jsonPath;
			else token.append(jsonPath, nextSlash - jsonPath);
			int32_t index = 0;
			if (json.is_array())
			{
				index = token.toLong();
				if (index >= json.size())
				{
					BBELOGLN("Index out of range in jsonElement: " << index);
					return;
				}
			}
			else
			{
				if (token != "%%%" && !json.contains(token.getRaw()))
				{
					BBELOGLN("Path not element of json in jsonElement: " << jsonPath);
					return;
				}
			}

			if constexpr (bbe::IsList<T>())
			{
				if (depth == 0)
				{
					value->clear();
				}
			}


			if (!nextSlash)
			{
				try
				{
					if (json.is_array())
					{
						if constexpr (bbe::IsList<T>())
						{
							value->add(json[index].get<typename T::SubType>());
						}
						else
						{
							*value = json[index].get<T>();
						}
					}
					else
					{
						if constexpr (bbe::IsList<T>())
						{
							value->add(json[token.getRaw()].get<typename T::SubType>());
						}
						else
						{
							*value = json[token.getRaw()].get<T>();
						}
					}
				}
				catch (std::exception& e)
				{
					BBELOGLN("Failed to get Value in jsonElement: " << e.what());
				}
			}
			else
			{
				if (token == "%%%")
				{
					for (auto it = json.begin(); it != json.end(); ++it)
					{
						jsonElement(value, *it, nextSlash + 1, depth + 1);
					}
				}
				else
				{
					jsonElement(value, json[token.getRaw()], nextSlash + 1, depth + 1);
				}
			}
		}

		template<typename... Pairs>
		void urlRequestJsonElements(const bbe::String& url, std::mutex* mutex, std::function<void()> andThen, Pairs&&... pairs)
		{
			auto request = urlRequest(url);
			if (request.responseCode == 200)
			{
				nlohmann::json json;
				try
				{
					json = nlohmann::json::parse(request.dataContainer.getRaw());
				}
				catch (std::exception& e)
				{
					BBELOGLN("Failed to parse json in urlRequestJsonElement: " << e.what());
					return;
				}

				// We do all the calculations on local copies first so that we can hold the lock as briefly as possible.
				auto computeLocalCopies = [&](auto&& pair)
					{
						using T = std::remove_pointer_t<decltype(pair.first)>;
						T local;
						jsonElement(&local, json, pair.second);
						return std::make_pair(pair.first, std::move(local)); // Pair of where to write the value later, and the value itself.
					};
				// Invoke that monster for all pairs...
				auto localValues = std::make_tuple(computeLocalCopies(pairs)...);

				std::unique_lock<std::mutex> ul;
				if (mutex) ul = std::unique_lock(*mutex);
				// I can feel the insanity rising...
				std::apply([](auto&&... p) { ((*(p.first) = std::move(p.second)), ...); }, localValues);

				if (andThen)
				{
					andThen();
				}
			}
			else
			{
				BBELOGLN("Request response wrong (" << request.responseCode << ") in urlRequestJsonElement" << " for URL: " << url);
			}
		}

		template<typename... Pairs>
		std::future<void> urlRequestJsonElementsAsync(const bbe::String& url, std::mutex* mutex, std::function<void()> andThen, Pairs&&... pairs)
		{
			return bbe::async(&urlRequestJsonElements<Pairs...>, url, mutex, andThen, pairs...);
		}

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
		SocketRequestXChaChaRet socketRequestXChaCha(const bbe::String& url, uint16_t port, const bbe::ByteBuffer& key, bool addTrailingNul = true, bool verbose = false);
		std::future<SocketRequestXChaChaRet> socketRequestXChaChaAsync(bbe::String /*copy*/ url, uint16_t port, const bbe::ByteBuffer& key, bool addTrailingNul = true, bool verbose = false);
		
		bbe::List<bbe::String> resolveDomain(const bbe::String& domain);
		bbe::List<bbe::String> resolveDomains(const bbe::List<bbe::String>& domains);
		bbe::String firewallBlockString(const bbe::List<bbe::String>& domains);
		void firewallBlockDomains(const bbe::String& ruleName, const bbe::List<bbe::String>& domains);
#endif
	}
}
#endif
