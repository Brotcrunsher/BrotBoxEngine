#include "BBE/Async.h"
#include "BBE/SimpleUrlRequest.h"
#include "BBE/Exceptions.h"
#include "BBE/List.h"

#ifdef BBE_ADD_CURL
#include "curl/curl.h"

struct CurlRaii
{
	CURL* curl = nullptr;

	CurlRaii()
	{
		curl = curl_easy_init();
		if (!curl)
		{
			throw bbe::IllegalStateException();
		}
	}

	~CurlRaii()
	{
		curl_easy_cleanup(curl);
	}

	CurlRaii(const CurlRaii& ) = delete;
	CurlRaii(      CurlRaii&&) = delete;
	CurlRaii& operator=(const CurlRaii& ) = delete;
	CurlRaii& operator=(      CurlRaii&&) = delete;

};

size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata)
{
	assert(size == 1); // Is documented to be always 1. This assertion protects us from API changes in the future.

	bbe::List<char> *dataContainer = (bbe::List<char>*)userdata;
	const size_t oldSize = dataContainer->getLength();
	dataContainer->addArray(ptr, nmemb);
	memcpy(dataContainer->getRaw() + oldSize, ptr, nmemb);
	return nmemb;
}

bbe::simpleUrlRequest::UrlRequestResult bbe::simpleUrlRequest::urlRequest(const bbe::String& url)
{
	CurlRaii c;
	CURL*& curl = c.curl;

	UrlRequestResult retVal;

	curl_easy_setopt(curl, CURLOPT_URL, url.getRaw());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &(retVal.dataContainer));

	CURLcode res;
	if ((res = curl_easy_perform(curl)) != CURLcode::CURLE_OK)
	{
		throw IllegalStateException();
	}

	retVal.dataContainer.add('\0');

	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &retVal.responseCode);

	return retVal;
}

std::future<bbe::simpleUrlRequest::UrlRequestResult> bbe::simpleUrlRequest::urlRequestAsync(const bbe::String& url)
{
	return bbe::async(&urlRequest, url);
}
#endif
