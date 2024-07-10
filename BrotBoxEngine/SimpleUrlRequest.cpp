#include "BBE/Async.h"
#include "BBE/SimpleUrlRequest.h"
#include "BBE/Exceptions.h"
#include "BBE/List.h"
#include "sodium.h"
#include "BBE/SimpleFile.h"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

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

bbe::simpleUrlRequest::UrlRequestResult bbe::simpleUrlRequest::urlRequest(const bbe::String& url, bool addTrailingNul, bool verbose)
{
	CurlRaii c;
	CURL*& curl = c.curl;

	UrlRequestResult retVal;

	curl_easy_setopt(curl, CURLOPT_URL, url.getRaw());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &(retVal.dataContainer));
	if(verbose) curl_easy_setopt(curl, CURLOPT_VERBOSE, TRUE);

	CURLcode res;
	if ((res = curl_easy_perform(curl)) != CURLcode::CURLE_OK)
	{
		throw IllegalStateException();
	}

	if (addTrailingNul)	retVal.dataContainer.add('\0');

	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &retVal.responseCode);

	return retVal;
}

std::future<bbe::simpleUrlRequest::UrlRequestResult> bbe::simpleUrlRequest::urlRequestAsync(const bbe::String& url, bool addTrailingNul, bool verbose)
{
	return bbe::async(&urlRequest, url, addTrailingNul, verbose);
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
		throw bbe::IllegalStateException();
	}
	const bbe::ByteBuffer key = bbe::simpleFile::readBinaryFile(pathToKeyFile);
	if (key.getLength() != crypto_aead_xchacha20poly1305_ietf_KEYBYTES)
	{
		throw bbe::IllegalStateException();
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

	if (messageLength > message.getLength()) throw bbe::IllegalStateException();

	if (addTrailingNul) message.add(0);
	return message;
}

#ifdef _WIN32
bbe::simpleUrlRequest::SocketRequestXChaChaRet bbe::simpleUrlRequest::socketRequestXChaCha(bbe::String /*copy*/ url, uint16_t port, const String& pathToKeyFile, bool addTrailingNul, bool verbose)
{
	WSADATA wsaData = { 0 };

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR)
	{
		throw bbe::IllegalStateException();
	}

	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET)
	{
		throw bbe::IllegalStateException();
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
	if (connect(sock, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		WSACleanup();
		return SocketRequestXChaChaRet{ SocketRequestXChaChaCode::HOST_NOT_REACHABLE };
	}
	
	bbe::List<char> data;
	char buffer[1024];
	while (true)
	{
		int recvLength = recv(sock, buffer, sizeof(buffer), 0);
		if (recvLength <= 0)
		{
			break;
		}
		data.addArray(buffer, recvLength);
	}
	auto decrypted = bbe::simpleUrlRequest::decryptXChaCha(data, pathToKeyFile);
	
	if (closesocket(sock) == SOCKET_ERROR)
	{
		throw bbe::IllegalStateException();
	}
	if (!decrypted)
	{
		return SocketRequestXChaChaRet{ SocketRequestXChaChaCode::FAILED_TO_DECRYPT };
	}

	WSACleanup();
	return SocketRequestXChaChaRet{ SocketRequestXChaChaCode::SUCCESS, *decrypted };
}
std::future<bbe::simpleUrlRequest::SocketRequestXChaChaRet> bbe::simpleUrlRequest::socketRequestXChaChaAsync(bbe::String url, uint16_t port, const String& pathToKeyFile, bool addTrailingNul, bool verbose)
{
	return bbe::async(&socketRequestXChaCha, url, port, pathToKeyFile, addTrailingNul, verbose);
}
#endif
#endif
