#include "BBE/Async.h"
#include "BBE/SimpleUrlRequest.h"
#include "BBE/Error.h"
#include "BBE/List.h"
#include "sodium.h"
#include "BBE/SimpleFile.h"
#include "BBE/Socket.h"

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
			bbe::Crash(bbe::Error::IllegalState);
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
	if(verbose) curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);

	CURLcode res;
	if ((res = curl_easy_perform(curl)) != CURLcode::CURLE_OK)
	{
		bbe::Crash(bbe::Error::IllegalState);
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
#endif
#endif
