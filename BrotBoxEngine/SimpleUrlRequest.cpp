#include "BBE/Async.h"
#include "BBE/SimpleUrlRequest.h"
#include "BBE/SimpleProcess.h"
#include "BBE/Error.h"
#include "BBE/List.h"
#include "sodium.h"
#include "BBE/SimpleFile.h"
#include "BBE/Socket.h"
#include "BBE/Logging.h"

#ifdef BBE_ADD_CURL
#include "curl/curl.h"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <netfw.h>
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "ws2_32.lib")

#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#endif

struct CurlRaii
{
	CURL* curl = nullptr;
	curl_slist* headers = nullptr;

	CurlRaii()
	{
		curl = curl_easy_init();
		if (!curl)
		{
			bbe::Crash(bbe::Error::IllegalState);
		}
	}

	~CurlRaii()
	{
		curl_slist_free_all(headers);
		curl_easy_cleanup(curl);
	}

	CurlRaii(const CurlRaii& ) = delete;
	CurlRaii(      CurlRaii&&) = delete;
	CurlRaii& operator=(const CurlRaii& ) = delete;
	CurlRaii& operator=(      CurlRaii&&) = delete;

};

static size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata)
{
	assert(size == 1); // Is documented to be always 1. This assertion protects us from API changes in the future.

	bbe::List<char> *dataContainer = (bbe::List<char>*)userdata;
	const size_t oldSize = dataContainer->getLength();
	dataContainer->addArray(ptr, nmemb);
	return nmemb;
}

bbe::simpleUrlRequest::UrlRequestResult bbe::simpleUrlRequest::urlRequest(const bbe::String& url, const bbe::List<bbe::String>& headerFields, const bbe::String& postData, bool addTrailingNul, bool verbose)
{
	CurlRaii c;
	CURL*& curl = c.curl;

	UrlRequestResult retVal;

	curl_easy_setopt(curl, CURLOPT_URL, url.getRaw());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &(retVal.dataContainer));
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 60L);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L);
	if (verbose) curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
	if (postData.getLength() > 0) {
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.getRaw());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, postData.getLengthBytes());
	}
	for (size_t i = 0; i < headerFields.getLength(); i++)
	{
		c.headers = curl_slist_append(c.headers, headerFields[i].getRaw());
	}
	if (c.headers)
	{
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, c.headers);
	}

	CURLcode res = curl_easy_perform(curl);

	if (res == CURLcode::CURLE_OK)
	{
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &retVal.responseCode);
	}
	else if (res == CURLcode::CURLE_OPERATION_TIMEDOUT)
	{
		retVal.responseCode = 523; // Origin Is Unreachable
	}
	else
	{
		bbe::String error = "Error (urlRequest): ";
		error += res;
		error += " For URL: " + url;
		bbe::Crash(bbe::Error::IllegalState, error.getRaw());
	}

	if (addTrailingNul)	retVal.dataContainer.add('\0');

	return retVal;
}

std::future<bbe::simpleUrlRequest::UrlRequestResult> bbe::simpleUrlRequest::urlRequestAsync(const bbe::String& url, const bbe::List<bbe::String>& headerFields, const bbe::String& postData, bool addTrailingNul, bool verbose)
{
	return bbe::async(&urlRequest, url, headerFields, postData, addTrailingNul, verbose);
}

bbe::simpleUrlRequest::UrlRequestResult bbe::simpleUrlRequest::urlFile(
	const bbe::String& url,
	const bbe::List<bbe::String>& headerFields,
	const std::map<bbe::String, bbe::String>& formFields,
	const bbe::ByteBuffer* fileData,
	const bbe::String& fileFieldName,
	const bbe::String& fileName,
	bool addTrailingNul,
	bool verbose)
{
	CurlRaii c;
	CURL*& curl = c.curl;

	UrlRequestResult retVal;

	curl_easy_setopt(curl, CURLOPT_URL, url.getRaw());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &(retVal.dataContainer));
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 120L);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120L);
	if (verbose) curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);

	for (size_t i = 0; i < headerFields.getLength(); i++)
	{
		c.headers = curl_slist_append(c.headers, headerFields[i].getRaw());
	}
	if (c.headers)
	{
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, c.headers);
	}

	curl_mime* mime = curl_mime_init(curl);

	for (const auto& field : formFields)
	{
		curl_mimepart* part = curl_mime_addpart(mime);
		curl_mime_name(part, field.first.getRaw());
		curl_mime_data(part, field.second.getRaw(), CURL_ZERO_TERMINATED);
	}

	if (fileData != nullptr)
	{
		curl_mimepart* part = curl_mime_addpart(mime);
		curl_mime_name(part, fileFieldName.getRaw());
		curl_mime_filename(part, fileName.getRaw());
		curl_mime_data(part, reinterpret_cast<const char*>(fileData->getRaw()), fileData->getLength());
		curl_mime_type(part, "audio/wav");
	}

	curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);


	CURLcode res = curl_easy_perform(curl);

	if (res == CURLcode::CURLE_OK)
	{
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &retVal.responseCode);
	}
	else if (res == CURLcode::CURLE_OPERATION_TIMEDOUT)
	{
		retVal.responseCode = 523; // Origin Is Unreachable
	}
	else
	{
		bbe::String error = "Error (urlFile): ";
		error += res;
		error += " For URL: " + url;
		bbe::Crash(bbe::Error::IllegalState, error.getRaw());
	}

	if (addTrailingNul) retVal.dataContainer.add('\0');

	curl_mime_free(mime);
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &retVal.responseCode);

	return retVal;
}



std::future<bbe::simpleUrlRequest::UrlRequestResult > bbe::simpleUrlRequest::urlFileAsync(
	const bbe::String& url,
	const bbe::List<bbe::String>& headerFields,
	const std::map<bbe::String, bbe::String>& formFields,
	const bbe::ByteBuffer* fileData,
	const bbe::String& fileFieldName,
	const bbe::String& fileName,
	bool addTrailingNul,
	bool verbose)
{
	return bbe::async(&urlFile, url, headerFields, formFields, fileData, fileFieldName, fileName, addTrailingNul, verbose);
}

void printArr(const unsigned char* arr, int len)
{
	for (int i = 0; i < len; i++)
	{
		printf("%02x", arr[i] & 0xff);
		if (i % 4 == 3) printf(" ");
	}
}

std::optional<bbe::List<char>> bbe::simpleUrlRequest::decryptXChaCha(const bbe::List<char>& data, const String& pathToKeyFile, bool addTrailingNul)
{
	if (sodium_init() < 0)
	{
		bbe::Crash(bbe::Error::IllegalState);
	}
	const bbe::ByteBuffer key = bbe::simpleFile::readBinaryFile(pathToKeyFile);
	if (key.getLength() != crypto_aead_xchacha20poly1305_ietf_KEYBYTES)
	{
		bbe::Crash(bbe::Error::IllegalState);
	}

	if (data.getLength() < crypto_aead_xchacha20poly1305_ietf_NPUBBYTES)
	{
		return std::nullopt;
	}
	const unsigned char* nonce = (const unsigned char*)data.getRaw();
	const unsigned char* ciphertext = nonce + crypto_aead_xchacha20poly1305_ietf_NPUBBYTES;
	const size_t ciphertextLenght = data.getLength() - crypto_aead_xchacha20poly1305_ietf_NPUBBYTES;

	printArr(nonce, crypto_aead_xchacha20poly1305_ietf_NPUBBYTES);
	printf("\n");

	bbe::List<char> message;
	message.resizeCapacityAndLengthUninit(ciphertextLenght - crypto_aead_xchacha20poly1305_ietf_ABYTES);
	unsigned long long messageLength;

	const int failure = crypto_aead_xchacha20poly1305_ietf_decrypt((unsigned char*)message.getRaw(), &messageLength,
		NULL,
		ciphertext, ciphertextLenght,
		nullptr,
		0,
		nonce, key.getRaw());

	if (failure) {
		return std::nullopt;
	}

	if (messageLength > message.getLength()) bbe::Crash(bbe::Error::IllegalState);

	if (addTrailingNul) message.add(0);
	return message;
}

#ifdef _WIN32
bbe::simpleUrlRequest::SocketRequestXChaChaRet bbe::simpleUrlRequest::socketRequestXChaCha(const bbe::String& url, uint16_t port, const String& pathToKeyFile, bool addTrailingNul, bool verbose)
{
	bbe::Socket socket(url, port);
	if (!socket.established())
	{
		return SocketRequestXChaChaRet{ SocketRequestXChaChaCode::HOST_NOT_REACHABLE };
	}
	bbe::List<char> data = socket.drain();

	auto decrypted = bbe::simpleUrlRequest::decryptXChaCha(data, pathToKeyFile);
	if (!decrypted)
	{
		return SocketRequestXChaChaRet{ SocketRequestXChaChaCode::FAILED_TO_DECRYPT };
	}

	return SocketRequestXChaChaRet{ SocketRequestXChaChaCode::SUCCESS, *decrypted };
}
std::future<bbe::simpleUrlRequest::SocketRequestXChaChaRet> bbe::simpleUrlRequest::socketRequestXChaChaAsync(bbe::String url, uint16_t port, const String& pathToKeyFile, bool addTrailingNul, bool verbose)
{
	return bbe::async(&socketRequestXChaCha, url, port, pathToKeyFile, addTrailingNul, verbose);
}
bbe::List<bbe::String> bbe::simpleUrlRequest::resolveDomain(const bbe::String& domain)
{
	bbe::List<bbe::String> retVal;
	ADDRINFOA info = { 0 };
	info.ai_family = AF_UNSPEC; // IPv4 + IPv6
	info.ai_socktype = SOCK_STREAM;
	info.ai_protocol = IPPROTO_TCP;

	ADDRINFOA* result = nullptr;
	int ret = GetAddrInfoA(domain.getRaw(), NULL, &info, &result);
	if (ret == 0 && result != NULL) {
		for (ADDRINFOA* ptr = result; ptr != NULL; ptr = ptr->ai_next) {
			char ipString[INET6_ADDRSTRLEN] = {};

			if (ptr->ai_family == AF_INET) {
				// IPv4
				SOCKADDR_IN* sockaddr_ipv4 = (SOCKADDR_IN*)ptr->ai_addr;
				InetNtopA(AF_INET, &sockaddr_ipv4->sin_addr, ipString, INET6_ADDRSTRLEN);
			}
			else if (ptr->ai_family == AF_INET6) {
				// IPv6
				SOCKADDR_IN6* sockaddr_ipv6 = (SOCKADDR_IN6*)ptr->ai_addr;
				InetNtopA(AF_INET6, &sockaddr_ipv6->sin6_addr, ipString, INET6_ADDRSTRLEN);
			}

			if (strlen(ipString) > 0) {
				retVal.add(ipString);
			}
		}
		FreeAddrInfoA(result);
	}

	return retVal;
}
bbe::List<bbe::String> bbe::simpleUrlRequest::resolveDomains(const bbe::List<bbe::String>& domains)
{
	bbe::List<bbe::String> retVal;
	for (size_t i = 0; i < domains.getLength(); i++)
	{
		bbe::List<bbe::String> cur = resolveDomain(domains[i]);
		for (size_t k = 0; k < cur.getLength(); k++)
		{
			retVal.addUnique(cur[k]);
		}
	}
	return retVal;
}
bbe::String bbe::simpleUrlRequest::firewallBlockString(const bbe::List<bbe::String>& domains)
{
	bbe::List<bbe::String> resolved = resolveDomains(domains);
	bbe::String retVal;
	for (size_t i = 0; i < resolved.getLength(); i++)
	{
		if (!retVal.isEmpty()) retVal += ",";
		retVal += resolved[i];
	}
	return retVal;
}
void bbe::simpleUrlRequest::firewallBlockDomains(const bbe::String& ruleName, const bbe::List<bbe::String>& domains)
{
	if (!bbe::simpleProcess::isRunAsAdmin())
	{
		bbe::Crash(bbe::Error::IllegalState, "Operation requires Admin/Elevation!");
	}

	INetFwPolicy2* policy = nullptr;
	HRESULT hr = CoCreateInstance(__uuidof(NetFwPolicy2), NULL, CLSCTX_INPROC_SERVER,
		__uuidof(INetFwPolicy2), (void**)&policy);
	if (FAILED(hr)) {
		// Error handling
		return;
	}

	INetFwRules* rules = nullptr;
	hr = policy->get_Rules(&rules);
	if (FAILED(hr)) {
		// Error handling
		policy->Release();
		return;
	}

	std::wstring ruleNameAsW = ruleName.toStdWString();
	if (domains.getLength() > 0) {
		const std::wstring remoteAddresses = firewallBlockString(domains).toStdWString();

		INetFwRule* rule = nullptr;
		hr = CoCreateInstance(__uuidof(NetFwRule), nullptr, CLSCTX_INPROC_SERVER,
			__uuidof(INetFwRule), (void**)&rule);
		if (SUCCEEDED(hr)) {
			rule->put_Name((wchar_t*)ruleNameAsW.c_str());
			rule->put_Action(NET_FW_ACTION_BLOCK);
			rule->put_Enabled(VARIANT_TRUE);
			wchar_t type[] = L"All";
			rule->put_InterfaceTypes(type);

			// Block both inbound and outbound traffic
			rule->put_Direction(NET_FW_RULE_DIR_OUT);

			// Set Protocol to Any
			rule->put_Protocol(NET_FW_IP_PROTOCOL_ANY);

			// Set valid RemoteAddresses
			rule->put_RemoteAddresses((wchar_t*)remoteAddresses.c_str());

			hr = rules->Add(rule);
			BBELOGLN("Firewall Add return: " << hr);

			rule->Release();
		}
	}
	else {
		INetFwRule* checkRule = nullptr;
		while (SUCCEEDED(rules->Item((wchar_t*)ruleNameAsW.c_str(), &checkRule)))
		{
			hr = rules->Remove((wchar_t*)ruleNameAsW.c_str());
			BBELOGLN("Firewall Remove return: " << hr);
		}
	}

	rules->Release();
	policy->Release();
}
#endif
#endif
