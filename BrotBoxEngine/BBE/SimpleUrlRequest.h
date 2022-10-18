#pragma once

#ifdef BBE_ADD_CURL
#include <future>

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

		UrlRequestResult urlRequest(const bbe::String& url);
		std::future<UrlRequestResult> urlRequestAsync(const bbe::String& url);
	}
}
#endif
